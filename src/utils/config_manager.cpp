#include "utils/config_manager.h"
#include "utils/logger.h"
#include <fstream>
#include <iostream>

using namespace nlohmann;

json ConfigManager::loadJsonConfig(const std::string& config_path) {
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            LOG_WARN_FMT("Config file not found: {}", config_path);
            return getDefaultPatterns();
        }

        json config;
        file >> config;
        file.close();

        LOG_INFO_FMT("Loaded config from: {}", config_path);
        return config;
    } catch (const std::exception& e) {
        LOG_WARN_FMT("Error loading config: {}", e.what());
        return getDefaultPatterns();
    }
}

bool ConfigManager::saveJsonConfig(const json& data, const std::string& output_path) {
    try {
        std::ofstream file(output_path);
        if (!file.is_open()) {
            LOG_ERROR_FMT("Cannot open file for writing: {}", output_path);
            return false;
        }

        file << data.dump(2) << std::endl;
        file.close();

        LOG_INFO_FMT("Saved config to: {}", output_path);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_FMT("Error saving config: {}", e.what());
        return false;
    }
}

json ConfigManager::getDefaultPatterns() {
    json patterns;

    auto add = [&](const std::string& name,
                   const std::string& regex,
                   const std::string& severity,
                   const std::string& description) {
        json p;
        p["regex"] = regex;
        p["severity"] = severity;
        p["description"] = description;
        patterns["patterns"][name] = p;
    };

    add("aws_key",
        R"((?i)(AKIA[0-9A-Z]{16}|aws[_-]?access[_-]?key[_-]?id|aws[_-]?secret[_-]?access[_-]?key))",
        "CRITICAL",
        "AWS Access Key or Secret");

    add("github_token",
        R"((?i)(github[_\-]?(token|pat|key)|ghp_[a-zA-Z0-9_]{36,255}))",
        "CRITICAL",
        "GitHub Personal Access Token");

    add("private_key",
        R"(-----BEGIN.*PRIVATE KEY.*-----)",
        "CRITICAL",
        "Private Key (RSA, DSA, EC)");

    add("password_plain",
        R"((?i)(password|passwd|pwd)\s*[:=]\s*['\"]?[^\s'"]+['\"]?)",
        "HIGH",
        "Password in plain text");

    add("database_url",
        R"((mongodb|mysql|postgres|redis)://[^\s]+)",
        "HIGH",
        "Database connection string");

    add("api_key",
        R"((?i)(api[_-]?key|apikey|api-secret|secret-key)\s*[:=]\s*['\"]?[\w\-\._]+['\"]?)",
        "HIGH",
        "Generic API Key");

    // high_entropy — без regex, только флаг
    json ent;
    ent["regex"] = nullptr;
    ent["severity"] = "MEDIUM";
    ent["description"] = "High entropy string (possible secret)";
    ent["use_entropy"] = true;
    ent["entropy_threshold"] = 4.5;
    patterns["patterns"]["high_entropy"] = ent;

    return patterns;
}

bool ConfigManager::configExists(const std::string& config_path) {
    std::ifstream file(config_path);
    return file.good();
}
