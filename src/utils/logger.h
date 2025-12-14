#pragma once

#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>

/**
 * @class Logger
 * @brief Синглтон для логирования (использует spdlog)
 */
class Logger {
public:
    /**
     * Получить инстанс логгера
     */
    static std::shared_ptr<spdlog::logger> getInstance();
    
    /**
     * Инициализировать логгер
     * @param log_file_path Опциональный путь до файла логов
     * @param level Уровень логирования
     */
    static void initialize(const std::string& log_file_path = "", 
                          spdlog::level::level_enum level = spdlog::level::info);
    
    /**
     * Установить уровень логирования
     */
    static void setLevel(spdlog::level::level_enum level);
    
private:
    static std::shared_ptr<spdlog::logger> instance;
};

// Удобные макросы
#define LOG_INFO(msg) Logger::getInstance()->info(msg)
#define LOG_DEBUG(msg) Logger::getInstance()->debug(msg)
#define LOG_WARN(msg) Logger::getInstance()->warn(msg)
#define LOG_ERROR(msg) Logger::getInstance()->error(msg)
#define LOG_CRITICAL(msg) Logger::getInstance()->critical(msg)

#define LOG_INFO_FMT(fmt, ...) Logger::getInstance()->info(fmt, __VA_ARGS__)
#define LOG_DEBUG_FMT(fmt, ...) Logger::getInstance()->debug(fmt, __VA_ARGS__)
#define LOG_WARN_FMT(fmt, ...) Logger::getInstance()->warn(fmt, __VA_ARGS__)
#define LOG_ERROR_FMT(fmt, ...) Logger::getInstance()->error(fmt, __VA_ARGS__)
#define LOG_CRITICAL_FMT(fmt, ...) Logger::getInstance()->critical(fmt, __VA_ARGS__)
