#include "utils/file_utils.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

std::string FileUtils::readFile(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    } catch (const std::exception&) {
        return "";
    }
}

bool FileUtils::writeFile(const std::string& file_path, const std::string& content) {
    try {
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }

        file << content;
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool FileUtils::fileExists(const std::string& file_path) {
    return fs::exists(file_path) && fs::is_regular_file(file_path);
}

bool FileUtils::directoryExists(const std::string& dir_path) {
    return fs::exists(dir_path) && fs::is_directory(dir_path);
}

bool FileUtils::createDirectory(const std::string& dir_path) {
    try {
        if (!fs::exists(dir_path)) {
            fs::create_directories(dir_path);
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string FileUtils::getFileExtension(const std::string& file_path) {
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos != std::string::npos && dot_pos != file_path.length() - 1) {
        std::string ext = file_path.substr(dot_pos + 1);
        // Lowercase
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
    return "";
}

std::string FileUtils::getFileName(const std::string& file_path) {
    size_t pos = file_path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return file_path.substr(pos + 1);
    }
    return file_path;
}

std::string FileUtils::getDirectoryPath(const std::string& file_path) {
    size_t pos = file_path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return file_path.substr(0, pos);
    }
    return ".";
}

std::string FileUtils::normalizePath(const std::string& path) {
    return fs::path(path).lexically_normal().string();
}

std::vector<std::string> FileUtils::listFilesInDirectory(const std::string& dir_path,
                                                         bool recursive) {
    std::vector<std::string> files;

    try {
        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
                if (fs::is_regular_file(entry)) {
                    files.push_back(entry.path().string());
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(dir_path)) {
                if (fs::is_regular_file(entry)) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception&) {
        // Ошибка при чтении директории
    }

    return files;
}

bool FileUtils::isAbsolutePath(const std::string& path) {
    return fs::path(path).is_absolute();
}

std::string FileUtils::toAbsolutePath(const std::string& path) {
    return fs::absolute(path).string();
}
