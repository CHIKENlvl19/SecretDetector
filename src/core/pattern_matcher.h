#pragma once

#include <string>
#include <vector>
#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @struct Pattern
 * @brief Структура для хранения одного паттерна поиска
 */
struct Pattern {
    std::string name;           ///< Имя паттерна (e.g., "aws_key")
    std::regex regex;           ///< Скомпилированный regex
    std::string severity;       ///< Уровень серьёзности (CRITICAL, HIGH, MEDIUM, LOW)
    std::string description;    ///< Описание паттерна
    bool use_entropy = false;   ///< Использовать энтропию анализ
    double entropy_threshold = 4.0; ///< Порог энтропии
    bool enabled = true;
};

/**
 * @struct Match
 * @brief Найденное совпадение (потенциальный секрет)
 */
struct Match {
    std::string file_path;      ///< Путь до файла
    int line_number;            ///< Номер строки (1-indexed)
    int column_number;          ///< Номер колонки
    std::string pattern_name;   ///< Имя сработавшего паттерна
    std::string matched_text;   ///< Найденный текст
    std::string severity;       ///< Уровень серьёзности
    std::string preview;        ///< Контекст (предпросмотр строки)
    double entropy;             ///< Рассчитанная энтропия (если использовалась)
    
    /**
     * Конвертировать Match в JSON
     */
    json to_json() const {
        json j;
        j["file"] = file_path;
        j["line"] = line_number;
        j["column"] = column_number;
        j["pattern"] = pattern_name;
        j["matched"] = matched_text;
        j["severity"] = severity;
        j["preview"] = preview;
        if (entropy > 0) {
            j["entropy"] = entropy;
        }
        return j;
    }
};

/**
 * @class PatternMatcher
 * @brief Класс для поиска паттернов в тексте
 */
class PatternMatcher {
public:
    PatternMatcher() = default;
    ~PatternMatcher() = default;
    
    /**
     * Загрузить паттерны из JSON конфигурации
     * @param config_path Путь до patterns.json
     * @return true если успешно, false если ошибка
     */
    bool loadPatterns(const std::string& config_path);

    /**
     * Загрузить паттерны из JSON объекта
     */
    bool loadFromJson(const json& patterns_json);

    /**
     * Найти все совпадения в тексте
     * @param content Содержимое файла
     * @param file_path Путь до файла (для отчета)
     * @return Вектор найденных совпадений
     */
    std::vector<Match> findMatches(const std::string& content, 
                                   const std::string& file_path) const;
    
    /**
     * Добавить кастомный паттерн
     */
    void addPattern(const Pattern& pattern);

    /**
     * Получить количество загруженных паттернов
     */
    size_t getPatternCount() const { return patterns.size(); }

    /**
     * Получить все паттерны
     */
    const std::vector<Pattern>& getPatterns() const { return patterns; }

private:
    std::vector<Pattern> patterns;
};
