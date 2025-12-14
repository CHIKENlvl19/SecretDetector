#include "core/file_scanner.h"
#include "utils/logger.h"
#include "utils/file_utils.h"
#include <filesystem>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>

namespace fs = std::filesystem;

FileScanner::FileScanner(ScanOptions opts) : options(std::move(opts)) {
    if (options.num_threads <= 0) {
        options.num_threads = std::thread::hardware_concurrency();
        if (options.num_threads == 0) options.num_threads = 4;
    }
}


std::vector<Match> FileScanner::scan(const PatternMatcher& matcher) {
    auto start_time = std::chrono::high_resolution_clock::now();
    statistics = ScanStatistics();

    std::vector<Match> all_matches;

    LOG_INFO_FMT("Starting scan of: {}", options.scan_path);
    LOG_DEBUG_FMT("Using {} threads", options.num_threads);

    // получить все файлы для сканирования
    auto files_to_scan = getFilesToScan();
    LOG_INFO_FMT("Found {} files to scan", files_to_scan.size());

    // загрузить gitignore паттерны если нужно
    std::vector<std::string> gitignore_patterns;
    if (options.respect_gitignore) {
        gitignore_patterns = loadGitignorePatterns();
    }

    // сканировать файлы
    size_t current = 0;
    std::mutex result_mutex;

    for (const auto& file_path : files_to_scan) {
        current++;

        // вызвать callback прогресса
        if (progress_callback) {
            progress_callback(current, files_to_scan.size());
        }

        // пропустить, если игнорируется
        if (options.respect_gitignore && 
            isIgnoredByGitignore(file_path, gitignore_patterns)) {
            LOG_DEBUG_FMT("Ignoring file: {}", file_path);
            continue;
        }

        // сканировать файл
        try {
            auto matches = scanFile(file_path, matcher);
            
            {
                std::lock_guard<std::mutex> lock(result_mutex);
                all_matches.insert(all_matches.end(), matches.begin(), matches.end());
                statistics.total_files_scanned++;
                statistics.total_matches_found += matches.size();
                
                // подсчитать по severity
                for (const auto& match : matches) {
                    if (match.severity == "CRITICAL") {
                        statistics.critical_count++;
                    } else if (match.severity == "HIGH") {
                        statistics.high_count++;
                    } else if (match.severity == "MEDIUM") {
                        statistics.medium_count++;
                    } else if (match.severity == "LOW") {
                        statistics.low_count++;
                    }
                }
            }
        } catch (const std::exception& e) {
            LOG_WARN_FMT("Error scanning file {}: {}", file_path, e.what());
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    statistics.scan_time_seconds = 
        std::chrono::duration<double>(end_time - start_time).count();

    LOG_INFO_FMT("Scan completed in {:.2f} seconds", statistics.scan_time_seconds);
    LOG_INFO_FMT("Found {} matches", statistics.total_matches_found);

    return all_matches;
}

std::vector<Match> FileScanner::scanFile(const std::string& file_path,
                                        const PatternMatcher& matcher) const {
    std::string content = FileUtils::readFile(file_path);
    if (content.empty()) {
        return {};
    }

    // подсчитать строки
    int line_count = std::count(content.begin(), content.end(), '\n') + 1;
    statistics.total_lines_scanned += line_count;

    return matcher.findMatches(content, file_path);
}

std::vector<std::string> FileScanner::getFilesToScan() {
    std::vector<std::string> files;

    try {
        if (!fs::exists(options.scan_path)) {
            LOG_ERROR_FMT("Path does not exist: {}", options.scan_path);
            return files;
        }

        if (options.recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(options.scan_path)) {
                if (fs::is_regular_file(entry) && shouldScanFile(entry.path().string())) {
                    files.push_back(entry.path().string());
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(options.scan_path)) {
                if (fs::is_regular_file(entry) && shouldScanFile(entry.path().string())) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR_FMT("Error reading directory: {}", e.what());
    }

    return files;
}

bool FileScanner::shouldScanFile(const std::string& file_path) const {
    std::string extension = FileUtils::getFileExtension(file_path);
    
    // список расширений для пропуска
    static const std::vector<std::string> skip_extensions = {
        // исполняемые и библиотеки
        ".exe", ".dll", ".so", ".dylib", ".a", ".o", ".obj", ".lib", ".bin", ".out",
        
        // архивы
        ".zip", ".tar", ".gz", ".bz2", ".7z", ".rar", ".xz", ".tgz",
        
        // картиночки
        ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".ico", ".svg", ".webp",
        ".tiff", ".tif", ".psd", ".ai", ".eps", ".raw",
        
        // видосики
        ".mp4", ".avi", ".mkv", ".mov", ".wmv", ".flv", ".webm", ".m4v",
        ".mpg", ".mpeg", ".3gp",
        
        // аудио
        ".mp3", ".wav", ".flac", ".ogg", ".m4a", ".aac", ".wma", ".opus",
        
        // документы (офисные форматы - бинарные оказывается)
        ".pdf", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx",
        ".odt", ".ods", ".odp", ".pages", ".numbers", ".key",
        
        // шрифты
        ".ttf", ".otf", ".woff", ".woff2", ".eot",
        
        // другие бинарные
        ".pyc", ".pyo", ".class", ".jar", ".war", ".ear",
        ".deb", ".rpm", ".dmg", ".pkg", ".msi", ".appimage",
        ".iso", ".img", ".dat",
        ".db", ".sqlite", ".sqlite3"
    };
    
    // преобразовать расширение в нижний регистр
    std::string lower_ext = extension;
    std::transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(), ::tolower);
    
    // проверить, есть ли расширение в списке пропускаемых
    for (const auto& skip_ext : skip_extensions) {
        if (lower_ext == skip_ext) {
            LOG_DEBUG_FMT("Skipping binary/media file: {}", file_path);
            return false;
        }
    }
    
    // пропустить файлы без расширения (часто бинарные)
    if (extension.empty()) {
        LOG_DEBUG_FMT("Skipping file without extension: {}", file_path);
        return false;
    }
    
    // проверить исключения пользователя
    for (const auto& exclude : options.exclude_patterns) {
        if (file_path.find(exclude) != std::string::npos) {
            return false;
        }
    }
    
    // псли указаны расширения для включения
    if (!options.include_extensions.empty()) {
        bool found = false;
        for (const auto& ext : options.include_extensions) {
            if (extension == ext) {
                found = true;
                break;
            }
        }
        return found;
    }
    
    return true;
}


std::vector<std::string> FileScanner::loadGitignorePatterns() {
    std::vector<std::string> patterns;

    try {
        std::string gitignore_path = options.scan_path + "/.gitignore";
        std::string content = FileUtils::readFile(gitignore_path);

        if (!content.empty()) {
            std::istringstream iss(content);
            std::string line;

            while (std::getline(iss, line)) {
                // пропустить комментарии и пустые строки
                if (line.empty() || line[0] == '#') {
                    continue;
                }

                // убрать пробелы
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);

                patterns.push_back(line);
            }

            LOG_DEBUG_FMT("Loaded {} gitignore patterns", patterns.size());
        }
    } catch (const std::exception& e) {
        LOG_DEBUG_FMT("No .gitignore found or error reading: {}", e.what());
    }

    return patterns;
}

bool FileScanner::isIgnoredByGitignore(const std::string& file_path,
                                     const std::vector<std::string>& gitignore_patterns) const {
    for (const auto& pattern : gitignore_patterns) {
        // Упрощенный matching (можно улучшить)
        if (file_path.find(pattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}
