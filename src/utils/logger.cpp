#include "utils/logger.h"
#include <iostream>
#include <memory>
#include <spdlog/sinks/rotating_file_sink.h>

std::shared_ptr<spdlog::logger> Logger::instance = nullptr;

std::shared_ptr<spdlog::logger> Logger::getInstance() {
    if (!instance) {
        initialize();
    }
    return instance;
}

void Logger::initialize(const std::string& log_file_path, 
                       spdlog::level::level_enum level) {
    try {
        std::vector<spdlog::sink_ptr> sinks;

        // Console sink (всегда есть)
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(level);
        sinks.push_back(console_sink);

        // File sink (если указан путь)
        if (!log_file_path.empty()) {
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                log_file_path, 10 * 1024 * 1024, 3); // 10MB, 3 files
            file_sink->set_level(level);
            sinks.push_back(file_sink);
        }

        // Создать logger с обоими sinks
        instance = std::make_shared<spdlog::logger>("secret_detector", sinks.begin(), sinks.end());
        instance->set_level(level);
        instance->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        
        spdlog::register_logger(instance);
    } catch (const spdlog::spdlog_ex& e) {
        std::cerr << "Log initialization failed: " << e.what() << std::endl;
    }
}

void Logger::setLevel(spdlog::level::level_enum level) {
    if (instance) {
        instance->set_level(level);
    }
}
