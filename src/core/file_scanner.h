#pragma once

#include <string>
#include <vector>
#include <memory>
#include "pattern_matcher.h"

/**
 * @struct ScanOptions
 * @brief Опции для сканирования
 */
struct ScanOptions {
    std::string scan_path;           ///< Путь до директории для сканирования
    bool recursive = true;           ///< Сканировать рекурсивно
    std::vector<std::string> include_extensions;  ///< Расширения для сканирования (если пусто - все)
    std::vector<std::string> exclude_patterns;    ///< Паттерны для исключения (e.g., "node_modules/*")
    bool respect_gitignore = true;   ///< Использовать .gitignore
    int num_threads = 0;             ///< Количество потоков (0 = автоматически)
};

/**
 * @struct ScanStatistics
 * @brief Статистика сканирования
 */
struct ScanStatistics {
    size_t total_files_scanned = 0;
    size_t total_matches_found = 0;
    size_t critical_count = 0;
    size_t high_count = 0;
    size_t medium_count = 0;
    size_t low_count = 0;
    double scan_time_seconds = 0.0;
    size_t total_lines_scanned = 0;
};

/**
 * @class FileScanner
 * @brief Основной класс для сканирования файлов
 */
class FileScanner {
public:
    explicit FileScanner(ScanOptions options);
    ~FileScanner() = default;
    
    /**
     * Начать сканирование директории
     * @param matcher Объект PatternMatcher с загруженными паттернами
     * @return Вектор найденных совпадений
     */
    std::vector<Match> scan(const PatternMatcher& matcher);
    
    /**
     * Сканировать отдельный файл
     * @param file_path Путь до файла
     * @param matcher Объект PatternMatcher
     * @return Вектор совпадений в этом файле
     */
    std::vector<Match> scanFile(const std::string& file_path,
                                const PatternMatcher& matcher) const;
    
    /**
     * Получить статистику последнего сканирования
     */
    const ScanStatistics& getStatistics() const { return statistics; }
    
    /**
     * Установить callback для прогресса
     * @param callback Функция: (current_file_index, total_files)
     */
    void setProgressCallback(std::function<void(size_t, size_t)> callback) {
        progress_callback = callback;
    }
    
private:
    ScanOptions options;
    mutable ScanStatistics statistics;
    std::function<void(size_t, size_t)> progress_callback;
    
    /**
     * Получить все файлы для сканирования
     */
    std::vector<std::string> getFilesToScan();
    
    /**
     * Проверить, нужно ли сканировать файл (по расширению, исключениям и т.д.)
     */
    bool shouldScanFile(const std::string& file_path) const;
    
    /**
     * Загрузить .gitignore паттерны
     */
    std::vector<std::string> loadGitignorePatterns();
    
    /**
     * Проверить, игнорируется ли файл по gitignore
     */
    bool isIgnoredByGitignore(const std::string& file_path,
                             const std::vector<std::string>& gitignore_patterns) const;
};
