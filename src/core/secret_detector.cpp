#include "core/secret_detector.h"
#include "core/file_scanner.h"
#include "utils/logger.h"
#include "utils/config_manager.h"
#include <chrono>

bool SecretDetector::initialize(const std::string& patterns_config_path) {
    LOG_INFO("Initializing Secret Detector...");
    
    // загрузить конфиг
    json patterns_json = ConfigManager::loadJsonConfig(patterns_config_path);
    
    // загрузить паттерны
    if (!matcher.loadFromJson(patterns_json)) {
        LOG_ERROR("Failed to load patterns");
        return false;
    }
    
    LOG_INFO_FMT("Loaded {} patterns", matcher.getPatternCount());
    return true;
}

ScanResult SecretDetector::scan(const ScanOptions& options) {
    ScanResult result;
    
    // создать сканер
    FileScanner scanner(options);
    
    // установить callback прогресса если нужно
    if (progress_callback) {
        scanner.setProgressCallback(progress_callback);
    }
    
    // выполнить сканирование
    result.matches = scanner.scan(matcher);
    result.statistics = scanner.getStatistics();
    
    // определить статус
    for (const auto& match : result.matches) {
        if (match.severity == "CRITICAL") {
            result.has_critical = true;
        } else if (match.severity == "HIGH") {
            result.has_high = true;
        }
    }
    
    return result;
}
