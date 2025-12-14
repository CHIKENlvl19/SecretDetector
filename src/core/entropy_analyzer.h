#pragma once

#include <string>
#include <vector>

/**
 * @class EntropyAnalyzer
 * @brief Анализатор энтропии строк для поиска "случайных" данных (возможные ключи)
 */
class EntropyAnalyzer {
public:
    EntropyAnalyzer() = default;
    ~EntropyAnalyzer() = default;

    /**
     * Рассчитать Shannon entropy для строки
     * Формула: H(X) = -sum(p(x) * log2(p(x)))
     * 
     * @param str Анализируемая строка
     * @return Значение энтропии (обычно 0-8)
     *         - 0-2: низкая энтропия (повторяющиеся символы)
     *         - 2-4: средняя энтропия (обычный текст)
     *         - 4-6: высокая энтропия (возможно зашифровано)
     *         - 6-8: очень высокая энтропия (явно случайные данные)
     */
    static double calculateEntropy(const std::string& str);

    /**
     * Проверить, имеет ли строка высокую энтропию
     * @param str Анализируемая строка
     * @param threshold Порог энтропии (по умолчанию 4.5)
     * @return true если энтропия >= threshold
     */
    static bool isHighEntropy(const std::string& str, double threshold = 4.5);

    /**
     * Получить статистику символов в строке
     * @param str Анализируемая строка
     * @return Пара: (количество уникальных символов, общее количество символов)
     */
    static std::pair<int, int> getCharacterStats(const std::string& str);
};
