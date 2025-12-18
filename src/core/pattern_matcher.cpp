#include "core/pattern_matcher.h"
#include "core/entropy_analyzer.h"
#include "utils/logger.h"
#include <fstream>
#include <sstream>

bool PatternMatcher::loadPatterns(const std::string& config_path) {
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            LOG_ERROR_FMT("Cannot open patterns config: {}", config_path);
            return false;
        }

        json patterns_json;
        file >> patterns_json;
        file.close();

        return loadFromJson(patterns_json);
    } catch (const std::exception& e) {
        LOG_ERROR_FMT("Error loading patterns: {}", e.what());
        return false;
    }
}

bool PatternMatcher::loadFromJson(const json& patterns_json) {
    try {
        patterns.clear();

        if (!patterns_json.contains("patterns")) {
            LOG_ERROR("JSON does not contain 'patterns' key");
            return false;
        }

        for (const auto& [name, pattern_data] : patterns_json["patterns"].items()) {
            try {
                Pattern pattern;
                pattern.name = name;
                pattern.description = pattern_data.value("description", "");
                pattern.severity = pattern_data.value("severity", "MEDIUM");

                // Загрузить regex если есть
                if (pattern_data.contains("regex") && !pattern_data["regex"].is_null()) {
                    std::string regex_str = pattern_data["regex"];

                    // убрать (?i) если есть (C++ regex не поддерживает inline flags)
                    if (regex_str.substr(0, 4) == "(?i)") {
                        regex_str = regex_str.substr(4);
                    }

                    // скомпилировать с флагом icase (case-insensitive по умолчанию)
                    pattern.regex = std::regex(regex_str, std::regex::icase | std::regex::ECMAScript);
                }

                // загрузить настройки энтропии если есть
                if (pattern_data.contains("use_entropy")) {
                    pattern.use_entropy = pattern_data["use_entropy"];
                }
                if (pattern_data.contains("entropy_threshold")) {
                    pattern.entropy_threshold = pattern_data["entropy_threshold"];
                }

                patterns.push_back(pattern);
                LOG_DEBUG_FMT("Loaded pattern: {}", name);
            } catch (const std::regex_error& e) {
                LOG_WARN_FMT("Invalid regex for pattern {}: {}", name, e.what());
            }
        }

        LOG_INFO_FMT("Successfully loaded {} patterns", patterns.size());
        return !patterns.empty();
    } catch (const std::exception& e) {
        LOG_ERROR_FMT("Error loading patterns from JSON: {}", e.what());
        return false;
    }
}


std::vector<Match> PatternMatcher::findMatches(const std::string& content,
                                                const std::string& file_path) const {
    std::vector<Match> matches;
    
    // применяем regex паттерны
    for (const auto& pattern : patterns) {
        // пропустить паттерны с энтропией (обработаем отдельно)
        if (pattern.use_entropy) {
            continue;
        }
        
        try {
            std::sregex_iterator iter(content.begin(), content.end(), pattern.regex);
            std::sregex_iterator end;
            
            while (iter != end) {
                Match match;
                match.file_path = file_path;
                match.pattern_name = pattern.name;
                match.matched_text = iter->str();
                match.severity = pattern.severity;
                
                // найти номер строки
                size_t pos = iter->position();
                match.line_number = 1 + std::count(content.begin(), content.begin() + pos, '\n');
                
                // получить preview (текущую строку)
                size_t line_start = content.rfind('\n', pos);
                if (line_start == std::string::npos) line_start = 0;
                else line_start++;
                
                size_t line_end = content.find('\n', pos);
                if (line_end == std::string::npos) line_end = content.length();
                
                match.preview = content.substr(line_start, line_end - line_start);
                match.column_number = pos - line_start + 1;
                match.entropy = 0.0;
                
                matches.push_back(match);
                ++iter;
            }
        } catch (const std::regex_error& e) {
            LOG_WARN_FMT("Invalid regex for pattern {}: {}", pattern.name, e.what());
        }
    }
    
    // отключить энтропию, пока что
    /*
    for (const auto& pattern : patterns) {
        if (pattern.use_entropy) {
            // Код энтропии
        }
    }
    */
    
    for (const auto& pattern : patterns) {
        if (!pattern.enabled) {  // Пропустить отключенные
            continue;
        }
    }

    return matches;
}

void PatternMatcher::addPattern(const Pattern& pattern) {
    patterns.push_back(pattern);
}

#include "core/entropy_analyzer.h"
