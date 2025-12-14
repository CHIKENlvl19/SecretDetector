#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @class ExportManager
 * @brief Менеджер для экспорта результатов в разные форматы
 */
class ExportManager {
public:
    ExportManager() = default;
    ~ExportManager() = default;

    /**
     * Экспортировать результаты в JSON
     * @param data JSON данные
     * @param output_path Путь до выходного файла
     * @return true если успешно
     */
    static bool exportToJson(const json& data, const std::string& output_path);

    /**
     * Экспортировать в CSV
     * @param data JSON данные результатов
     * @param output_path Путь до выходного файла
     * @return true если успешно
     */
    static bool exportToCsv(const json& data, const std::string& output_path);

    /**
     * Экспортировать в HTML отчет
     * @param data JSON данные результатов
     * @param output_path Путь до выходного файла
     * @return true если успешно
     */
    static bool exportToHtml(const json& data, const std::string& output_path);

    /**
     * Экспортировать в обычный текст
     * @param data JSON данные результатов
     * @param output_path Путь до выходного файла
     * @return true если успешно
     */
    static bool exportToText(const json& data, const std::string& output_path);
};
