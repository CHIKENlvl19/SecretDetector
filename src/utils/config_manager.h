#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @class ConfigManager
 * @brief Менеджер для работы с конфигурационными файлами
 */
class ConfigManager {
public:
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    /**
     * Загрузить JSON конфиг
     * @param config_path Путь до файла
     * @return JSON объект или пустой json если ошибка
     */
    static json loadJsonConfig(const std::string& config_path);
    
    /**
     * Сохранить JSON в файл
     * @param data JSON данные
     * @param output_path Путь до файла
     * @return true если успешно
     */
    static bool saveJsonConfig(const json& data, const std::string& output_path);
    
    /**
     * Получить встроенный конфиг по умолчанию
     * @return JSON с дефолтными паттернами
     */
    static json getDefaultPatterns();
    
    /**
     * Проверить, существует ли конфиг файл
     */
    static bool configExists(const std::string& config_path);
};
