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

    // разделить содержимое на строки
    std::istringstream iss(content);
    std::string line;
    int line_number = 0;

    while (std::getline(iss, line)) {
        line_number++;

        // применить каждый паттерн
        for (const auto& pattern : patterns) {
            // regex-based matching
                std::smatch match;
                std::string::const_iterator search_start(line.cbegin());

                while (std::regex_search(search_start, line.cend(), match, pattern.regex)) {
                    Match result;
                    result.file_path = file_path;
                    result.line_number = line_number;
                    result.column_number = std::distance(line.cbegin(), match[0].first) + 1;
                    result.pattern_name = pattern.name;
                    result.matched_text = match[0];
                    result.severity = pattern.severity;
                    result.preview = line;

                    // ограничить вывод "matched_text" для конфиденциальности
                    if (result.matched_text.length() > 50) {
                        result.matched_text = result.matched_text.substr(0, 47) + "...";
                    }

                    // ограничить preview
                    if (result.preview.length() > 100) {
                        result.preview = result.preview.substr(0, 97) + "...";
                    }

                    matches.push_back(result);
                    search_start = match[0].second;
                }

            // Entropy-based matching
            if (pattern.use_entropy) {
                // попробовать найти потенциальные ключи по энтропии
                // это может включать поиск строк, похожих на ключи
                // упрощенная версия: ищем слова с большим количеством символов
                std::istringstream word_iss(line);
                std::string word;
                int column = 0;

                while (word_iss >> word) {
                    column = line.find(word, column);
                    if (column != std::string::npos) {
                        if (word.length() > 20) { // Длина ключа обычно > 20
                            double entropy = EntropyAnalyzer::calculateEntropy(word);
                            if (entropy >= pattern.entropy_threshold) {
                                Match result;
                                result.file_path = file_path;
                                result.line_number = line_number;
                                result.column_number = column + 1;
                                result.pattern_name = pattern.name + "_entropy";
                                result.matched_text = word.substr(0, 50) + (word.length() > 50 ? "..." : "");
                                result.severity = pattern.severity;
                                result.preview = line;
                                result.entropy = entropy;
                                
                                matches.push_back(result);
                            }
                        }
                        column += word.length();
                    }
                }
            }
        }
    }

    return matches;
}

void PatternMatcher::addPattern(const Pattern& pattern) {
    patterns.push_back(pattern);
}

#include "core/entropy_analyzer.h"
