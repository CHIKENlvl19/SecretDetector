#pragma once

#include <string>
#include <vector>

/**
 * @struct CLIOptions
 * @brief Опции командной строки
 */
struct CLIOptions {
    std::string scan_path;              ///< Путь для сканирования
    std::string output_path;            ///< Путь для вывода результатов
    std::string config_path;            ///< Путь до конфига паттернов
    std::string format = "text";        ///< Формат вывода (text, json, csv, html)
    bool verbose = false;               ///< Подробное логирование
    bool strict = false;                ///< Строгий режим (блокировать при любых секретах)
    bool recursive = true;              ///< Сканировать рекурсивно
    bool help = false;                  ///< Показать справку
    bool version = false;               ///< Показать версию
    bool respect_gitignore = true;      ///< Использовать .gitignore
    std::vector<std::string> exclude_patterns;  ///< Паттерны для исключения
    std::vector<std::string> include_extensions;  ///< Расширения для включения
    int num_threads = 0;                ///< Количество потоков (0 = авто)
};

/**
 * @class CLIParser
 * @brief Парсер аргументов командной строки
 */
class CLIParser {
public:
    CLIParser() = default;
    ~CLIParser() = default;
    
    /**
     * Распарсить аргументы командной строки
     * @param argc Количество аргументов
     * @param argv Массив аргументов
     * @return Заполненный CLIOptions или выход с ошибкой
     */
    static CLIOptions parse(int argc, char* argv[]);
    
    /**
     * Показать справку
     */
    static void printHelp();
    
    /**
     * Показать версию
     */
    static void printVersion();
    
    /**
     * Валидировать опции
     * @param options Опции для валидации
     * @return true если валидны
     */
    static bool validate(const CLIOptions& options);
};
