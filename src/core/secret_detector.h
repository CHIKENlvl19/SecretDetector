#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "pattern_matcher.h"
#include "file_scanner.h"

using json = nlohmann::json;

/**
 * @struct ScanResult
 * @brief Полный результат сканирования
 */
struct ScanResult {
    std::vector<Match> matches;
    ScanStatistics statistics;
    bool has_critical = false;
    bool has_high = false;

    /**
     * Получить обобщённый статус
     * @return 0 - no issues, 1 - LOW/MEDIUM, 2 - HIGH, 3 - CRITICAL
     */
    int getStatusCode() const {
        if (has_critical) return 3;
        if (has_high) return 2;
        if (statistics.total_matches_found > 0) return 1;
        return 0;
    }
    
    /**
     * Конвертировать результат в JSON
     */
nlohmann::json to_json() const {
    nlohmann::json j;

    j["scan_summary"] = {
        {"total_matches", statistics.total_matches_found},
        {"total_files", statistics.total_files_scanned},
        {"total_lines", statistics.total_lines_scanned},
        {"scan_time_seconds", statistics.scan_time_seconds},
        {"has_critical", has_critical},
        {"has_high", has_high}
    };

    j["severity_breakdown"] = {
        {"critical", statistics.critical_count},
        {"high", statistics.high_count},
        {"medium", statistics.medium_count},
        {"low", statistics.low_count}
    };

    // создать массив результатов
    nlohmann::json matches_array = nlohmann::json::array();
        for (const auto& match : matches) {
            nlohmann::json match_obj;  // Создать объект для каждого match
            match_obj["file_path"] = match.file_path;
            match_obj["line_number"] = match.line_number;
            match_obj["column_number"] = match.column_number;
            match_obj["severity"] = match.severity;
            match_obj["pattern_name"] = match.pattern_name;
            match_obj["matched_text"] = match.matched_text;
            match_obj["preview"] = match.preview;
            if (match.entropy > 0) {
                match_obj["entropy"] = match.entropy;
            }
            matches_array.push_back(match_obj);
        }
        j["results"] = matches_array;

        // добавить статистику
        j["statistics"] = {
            {"total_files_scanned", statistics.total_files_scanned},
            {"total_matches_found", statistics.total_matches_found},
            {"critical_count", statistics.critical_count},
            {"high_count", statistics.high_count},
            {"medium_count", statistics.medium_count},
            {"low_count", statistics.low_count},
            {"scan_time_seconds", statistics.scan_time_seconds},
            {"total_lines_scanned", statistics.total_lines_scanned}
        };

        return j;
    }

};

/**
 * @class SecretDetector
 * @brief Главный класс, координирующий сканирование
 */
class SecretDetector {
public:
    SecretDetector() = default;
    ~SecretDetector() = default;

    /**
     * Инициализировать детектор с паттернами
     * @param patterns_config_path Путь до patterns.json
     * @return true если успешно
     */
    bool initialize(const std::string& patterns_config_path);

    /**
     * Выполнить полное сканирование
     * @param options Опции сканирования
     * @return Результат сканирования
     */
    ScanResult scan(const ScanOptions& options);

    /**
     * Установить callback для прогресса
     */
    void setProgressCallback(std::function<void(size_t, size_t)> callback) {
        progress_callback = callback;
    }

private:
    PatternMatcher matcher;
    std::function<void(size_t, size_t)> progress_callback;
};
