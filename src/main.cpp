#include "cli/cli_parser.h"
#include "core/secret_detector.h"
#include "utils/logger.h"
#include "utils/export_manager.h"
#include "utils/file_utils.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>

// цветной вывод
const char* RED = "\033[1;31m";
const char* GREEN = "\033[1;32m";
const char* YELLOW = "\033[1;33m";
const char* BLUE = "\033[1;34m";
const char* RESET = "\033[0m";

void printSummary(const ScanResult& result, bool strict_mode) {
    std::cout << "\n" << BLUE << std::string(50, '=') << RESET << "\n";
    std::cout << "\t\tSCAN SUMMARY\n";
    std::cout << BLUE << std::string(50, '=') << RESET << "\n";

    const auto& stats = result.statistics;
    std::cout << "Files scanned:    " << stats.total_files_scanned << "\n";
    std::cout << "Total lines:      " << stats.total_lines_scanned << "\n";
    std::cout << "Total matches:    " << stats.total_matches_found << "\n";

    if (stats.critical_count > 0) {
        std::cout << RED << "  CRITICAL: " << stats.critical_count << RESET << "\n";
    } else {
        std::cout << "  CRITICAL: " << stats.critical_count << "\n";
    }

    if (stats.high_count > 0) {
        std::cout << YELLOW << "  HIGH: " << stats.high_count << RESET << "\n";
    } else {
        std::cout << "  HIGH: " << stats.high_count << "\n";
    }

    std::cout << "  MEDIUM: " << stats.medium_count << "\n";
    std::cout << "  LOW: " << stats.low_count << "\n";

    std::cout << "Scan time:        " << std::fixed << std::setprecision(2) 
              << stats.scan_time_seconds << "s\n";

    std::cout << "\n" << BLUE << std::string(50, '=') << RESET << "\n";

    // код выхода
    int exit_code = 0;
    if (result.has_critical || (strict_mode && result.statistics.total_matches_found > 0)) {
        std::cout << RED << "SCAN FAILED: Secrets detected!" << RESET << "\n";
        exit_code = 1;
    } else if (result.has_high) {
        std::cout << YELLOW << "WARNING: High severity matches found" << RESET << "\n";
        exit_code = 0; // Don't fail, just warn
    } else if (result.statistics.total_matches_found > 0) {
        std::cout << YELLOW << "Info: Some matches found (LOW/MEDIUM severity)" << RESET << "\n";
        exit_code = 0;
    } else {
        std::cout << GREEN << "SCAN PASSED: No secrets detected!" << RESET << "\n";
        exit_code = 0;
    }

    std::cout << "\n";

    // топ находок
    if (!result.matches.empty()) {
        std::cout << BLUE << "TOP FINDINGS (max 10):" << RESET << "\n";
        int count = 0;
        for (const auto& match : result.matches) {
            if (count >= 10) break;
            if (match.severity == "CRITICAL" || match.severity == "HIGH") {
                std::cout << "  [" << match.severity << "] "
                         << match.file_path << ":" << match.line_number
                         << " - " << match.pattern_name << "\n";
                count++;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // парсинг аргуметов ввода
    CLIOptions options = CLIParser::parse(argc, argv);

    // справка
    if (options.help) {
        CLIParser::printHelp();
        return 0;
    }

    // версия ПО
    if (options.version) {
        CLIParser::printVersion();
        return 0;
    }

    // логгер
    spdlog::level::level_enum log_level = options.verbose 
        ? spdlog::level::debug 
        : spdlog::level::info;
    Logger::initialize("", log_level);

    LOG_INFO("Secret Detector started");
    LOG_DEBUG_FMT("Scan path: {}", options.scan_path);
    LOG_DEBUG_FMT("Output format: {}", options.format);

    // проверка опций
    if (!CLIParser::validate(options)) {
        LOG_ERROR("Invalid options");
        std::cerr << "Use --help for usage information\n";
        return 1;
    }

    // сам детектор
    SecretDetector detector;

    std::string config_path = options.config_path.empty() 
        ? "/etc/secret_detector/patterns.json" 
        : options.config_path;

    // поиск конфига
    if (!FileUtils::fileExists(config_path)) {
        std::vector<std::string> possible_paths = {
            "/opt/secret-detector/config/patterns.json"
        };

        config_path = "";
        for (const auto& path : possible_paths) {
            if (FileUtils::fileExists(path)) {
                config_path = path;
                break;
            }
        }

        if (config_path.empty()) {
            LOG_WARN("Config file not found, using built-in patterns");
        }
    }

    if (!detector.initialize(config_path)) {
        LOG_ERROR("Failed to initialize detector");
        return 1;
    }

    // опуии сканирования
    ScanOptions scan_options;
    scan_options.scan_path = options.scan_path;
    scan_options.recursive = options.recursive;
    scan_options.respect_gitignore = options.respect_gitignore;
    scan_options.num_threads = options.num_threads;
    scan_options.exclude_patterns = options.exclude_patterns;
    scan_options.include_extensions = options.include_extensions;

    // прогресс сканирования
    detector.setProgressCallback([](size_t current, size_t total) {
        int percent = (current * 100) / total;
        std::cout << "\rProgress: " << current << "/" << total 
                 << " (" << percent << "%)" << std::flush;
    });

    // начало сканирования
    LOG_INFO("Starting scan...");
    ScanResult result = detector.scan(scan_options);

    std::cout << "\r" << std::string(50, ' ') << "\r"; // очистка линии прогресса

    // экспорт отчета
    if (!options.output_path.empty()) {
        LOG_INFO_FMT("Exporting results to: {}", options.output_path);

        bool export_success = false;
        json output = result.to_json();

        if (options.format == "json") {
            export_success = ExportManager::exportToJson(output, options.output_path);
        } else if (options.format == "csv") {
            export_success = ExportManager::exportToCsv(output, options.output_path);
        } else if (options.format == "html") {
            export_success = ExportManager::exportToHtml(output, options.output_path);
        } else if (options.format == "text") {
            export_success = ExportManager::exportToText(output, options.output_path);
        }

        if (!export_success) {
            LOG_ERROR("Failed to export results");
            return 1;
        }
    } else {
        // печатаем в консоль
        if (options.format == "json") {
            std::cout << result.to_json().dump(2) << "\n";
        } else if (options.format == "csv") {
            std::cout << "File,Line,Column,Pattern,Severity,Preview\n";
            for (const auto& match : result.matches) {
                std::cout << "\"" << match.file_path << "\","
                         << match.line_number << ","
                         << match.column_number << ","
                         << "\"" << match.pattern_name << "\","
                         << "\"" << match.severity << "\","
                         << "\"" << match.preview << "\"\n";
            }
        } else {
            // текстовый формат по умолчанию
            if (!result.matches.empty()) {
                std::cout << "\nMatches found:\n";
                for (const auto& match : result.matches) {
                    std::cout << match.file_path << ":"
                             << match.line_number << " ["
                             << match.severity << "] "
                             << match.pattern_name << "\n";
                    std::cout << "  " << match.preview << "\n\n";
                }
            }
        }
    }

    // заключение
    printSummary(result, options.strict);

    // код возврата
    if (result.has_critical) {
        return 1;
    } else if (options.strict && result.statistics.total_matches_found > 0) {
        return 1;
    }

    return 0;
}
