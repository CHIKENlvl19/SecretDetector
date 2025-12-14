#include "utils/export_manager.h"
#include "utils/logger.h"
#include <fstream>
#include <sstream>
#include <iomanip>

bool ExportManager::exportToJson(const json& data, const std::string& output_path) {
    try {
        std::ofstream file(output_path);
        if (!file.is_open()) {
            LOG_ERROR_FMT("Cannot open file for writing: {}", output_path);
            return false;
        }
        
        file << data.dump(2) << std::endl;
        file.close();
        
        LOG_INFO_FMT("Exported to JSON: {}", output_path);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_FMT("Error exporting to JSON: {}", e.what());
        return false;
    }
}

bool ExportManager::exportToCsv(const json& data, const std::string& output_path) {
    try {
        std::ofstream file(output_path);
        if (!file.is_open()) {
            LOG_ERROR_FMT("Cannot open file for writing: {}", output_path);
            return false;
        }
        
        // CSV header
        file << "File,Line,Column,Pattern,Severity,Preview\n";
        
        // CSV body
        if (data.contains("results") && data["results"].is_array()) {
            for (const auto& result : data["results"]) {
                // Escape CSV
                std::string file_val = result.value("file", "");
                std::string pattern_val = result.value("pattern", "");
                std::string severity_val = result.value("severity", "");
                std::string preview_val = result.value("preview", "");
                
                // Replace quotes and newlines
                for (auto& c : preview_val) {
                    if (c == '"') c = '\'';
                    if (c == '\n' || c == '\r') c = ' ';
                }
                
                file << "\"" << file_val << "\","
                     << result.value("line", 0) << ","
                     << result.value("column", 0) << ","
                     << "\"" << pattern_val << "\","
                     << "\"" << severity_val << "\","
                     << "\"" << preview_val << "\"\n";
            }
        }
        
        file.close();
        LOG_INFO_FMT("Exported to CSV: {}", output_path);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_FMT("Error exporting to CSV: {}", e.what());
        return false;
    }
}

bool ExportManager::exportToHtml(const json& data, const std::string& output_path) {
    try {
        std::ofstream file(output_path);
        if (!file.is_open()) {
            LOG_ERROR_FMT("Cannot open file for writing: {}", output_path);
            return false;
        }
        
        // HTML header
        file << R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Secret Detector Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        h1 { color: #333; }
        .summary { background: white; padding: 15px; border-radius: 5px; margin-bottom: 20px; }
        .critical { color: #d32f2f; font-weight: bold; }
        .high { color: #f57c00; font-weight: bold; }
        .medium { color: #fbc02d; font-weight: bold; }
        .low { color: #388e3c; font-weight: bold; }
        table { width: 100%; border-collapse: collapse; background: white; }
        th { background: #333; color: white; padding: 12px; text-align: left; }
        td { padding: 10px; border-bottom: 1px solid #ddd; }
        tr:hover { background: #f9f9f9; }
        .result-item { margin-bottom: 10px; padding: 10px; border-left: 4px solid #ddd; background: white; }
        .result-item.critical { border-left-color: #d32f2f; }
        .result-item.high { border-left-color: #f57c00; }
        .result-item.medium { border-left-color: #fbc02d; }
        .result-item.low { border-left-color: #388e3c; }
    </style>
</head>
<body>
    <h1>🔍 Secret Detector Report</h1>
)";
        
        // Summary
        file << "    <div class=\"summary\">\n";
        if (data.contains("statistics")) {
            const auto& stats = data["statistics"];
            file << "        <p><strong>Scan Statistics:</strong></p>\n";
            file << "        <ul>\n";
            file << "            <li>Files Scanned: " << stats.value("total_files_scanned", 0) << "</li>\n";
            file << "            <li>Total Matches: " << stats.value("total_matches_found", 0) << "</li>\n";
            file << "            <li><span class=\"critical\">Critical: " 
                 << stats.value("critical_count", 0) << "</span></li>\n";
            file << "            <li><span class=\"high\">High: " 
                 << stats.value("high_count", 0) << "</span></li>\n";
            file << "            <li><span class=\"medium\">Medium: " 
                 << stats.value("medium_count", 0) << "</span></li>\n";
            file << "            <li><span class=\"low\">Low: " 
                 << stats.value("low_count", 0) << "</span></li>\n";
            file << "        </ul>\n";
        }
        file << "    </div>\n";
        
        // Results
        if (data.contains("results") && data["results"].is_array()) {
            file << "    <h2>Findings</h2>\n";
            for (const auto& result : data["results"]) {
                std::string severity = result.value("severity", "");
                std::string severity_lower = severity;
                std::transform(severity_lower.begin(), severity_lower.end(), 
                             severity_lower.begin(), ::tolower);
                
                file << "    <div class=\"result-item " << severity_lower << "\">\n";
                file << "        <strong>" << severity << " - " << result.value("pattern", "") << "</strong><br>\n";
                file << "        <small>File: " << result.value("file", "") << ":"
                     << result.value("line", 0) << "</small><br>\n";
                file << "        <code>" << result.value("preview", "") << "</code>\n";
                file << "    </div>\n";
            }
        }
        
        file << R"(
</body>
</html>)";
        
        file.close();
        LOG_INFO_FMT("Exported to HTML: {}", output_path);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_FMT("Error exporting to HTML: {}", e.what());
        return false;
    }
}

bool ExportManager::exportToText(const json& data, const std::string& output_path) {
    try {
        std::ofstream file(output_path);
        if (!file.is_open()) {
            LOG_ERROR_FMT("Cannot open file for writing: {}", output_path);
            return false;
        }
        
        file << "Secret Detector Report\n";
        file << "======================\n\n";
        
        // Statistics
        if (data.contains("statistics")) {
            file << "STATISTICS:\n";
            const auto& stats = data["statistics"];
            file << "  Files scanned: " << stats.value("total_files_scanned", 0) << "\n";
            file << "  Total matches: " << stats.value("total_matches_found", 0) << "\n";
            file << "  CRITICAL: " << stats.value("critical_count", 0) << "\n";
            file << "  HIGH: " << stats.value("high_count", 0) << "\n";
            file << "  MEDIUM: " << stats.value("medium_count", 0) << "\n";
            file << "  LOW: " << stats.value("low_count", 0) << "\n";
            file << "  Scan time: " << stats.value("scan_time_seconds", 0.0) << "s\n\n";
        }
        
        // Results
        if (data.contains("results") && data["results"].is_array()) {
            file << "FINDINGS:\n";
            file << "---------\n";
            for (const auto& result : data["results"]) {
                file << "[" << result.value("severity", "") << "] "
                     << result.value("file_path", "") << ":"
                     << result.value("line_number", 0) << "\n";
                file << "  Pattern: " << result.value("pattern_name", "") << "\n";
                file << "  Preview: " << result.value("preview", "") << "\n\n";
            }
        }
        
        file.close();
        LOG_INFO_FMT("Exported to Text: {}", output_path);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_FMT("Error exporting to Text: {}", e.what());
        return false;
    }
}
