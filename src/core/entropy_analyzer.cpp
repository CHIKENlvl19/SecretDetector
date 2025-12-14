#include "core/entropy_analyzer.h"
#include <cmath>
#include <unordered_map>
#include <algorithm>

double EntropyAnalyzer::calculateEntropy(const std::string& str) {
    if (str.empty()) {
        return 0.0;
    }
    
    // рассчитать частоту каждого символа
    std::unordered_map<char, int> frequencies;
    for (char c : str) {
        frequencies[c]++;
    }
    
    // рассчитать энтропию: H(X) = -sum(p(x) * log2(p(x)))
    double entropy = 0.0;
    const double length = static_cast<double>(str.length());
    
    for (const auto& pair : frequencies) {
        double probability = pair.second / length;
        if (probability > 0) {
            entropy -= probability * std::log2(probability);
        }
    }
    
    return entropy;
}

bool EntropyAnalyzer::isHighEntropy(const std::string& str, double threshold) {
    return calculateEntropy(str) >= threshold;
}

std::pair<int, int> EntropyAnalyzer::getCharacterStats(const std::string& str) {
    if (str.empty()) {
        return {0, 0};
    }
    
    std::unordered_map<char, bool> unique_chars;
    for (char c : str) {
        unique_chars[c] = true;
    }
    
    return {static_cast<int>(unique_chars.size()), static_cast<int>(str.length())};
}
