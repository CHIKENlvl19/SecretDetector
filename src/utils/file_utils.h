#pragma once

#include <string>
#include <vector>

/**
 * @class FileUtils
 * @brief Вспомогательные функции для работы с файлами
 */
class FileUtils {
public:
    FileUtils() = default;
    ~FileUtils() = default;
    
    /**
     * Прочитать содержимое файла
     * @param file_path Путь до файла
     * @return Содержимое файла или пустая строка если ошибка
     */
    static std::string readFile(const std::string& file_path);
    
    /**
     * Написать содержимое в файл
     * @param file_path Путь до файла
     * @param content Содержимое для записи
     * @return true если успешно
     */
    static bool writeFile(const std::string& file_path, const std::string& content);
    
    /**
     * Проверить, существует ли файл
     * @param file_path Путь до файла
     * @return true если существует
     */
    static bool fileExists(const std::string& file_path);
    
    /**
     * Проверить, существует ли директория
     * @param dir_path Путь до директории
     * @return true если существует
     */
    static bool directoryExists(const std::string& dir_path);
    
    /**
     * Создать директорию (если не существует)
     * @param dir_path Путь до директории
     * @return true если успешно
     */
    static bool createDirectory(const std::string& dir_path);
    
    /**
     * Получить расширение файла
     * @param file_path Путь до файла
     * @return Расширение (e.g., "cpp", "json")
     */
    static std::string getFileExtension(const std::string& file_path);
    
    /**
     * Получить имя файла без пути
     * @param file_path Полный путь
     * @return Имя файла
     */
    static std::string getFileName(const std::string& file_path);
    
    /**
     * Получить директорию из пути
     * @param file_path Полный путь
     * @return Путь до директории
     */
    static std::string getDirectoryPath(const std::string& file_path);
    
    /**
     * Нормализировать путь (удалить ../ и т.д.)
     * @param path Путь для нормализации
     * @return Нормализированный путь
     */
    static std::string normalizePath(const std::string& path);
    
    /**
     * Получить все файлы в директории
     * @param dir_path Путь до директории
     * @param recursive Сканировать рекурсивно
     * @return Вектор путей до файлов
     */
    static std::vector<std::string> listFilesInDirectory(const std::string& dir_path,
                                                         bool recursive = true);
    
    /**
     * Проверить, является ли путь абсолютным
     */
    static bool isAbsolutePath(const std::string& path);
    
    /**
     * Преобразовать в абсолютный путь
     */
    static std::string toAbsolutePath(const std::string& path);
};
