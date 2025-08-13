#ifndef SEARCHSERVER_H
#define SEARCHSERVER_H

#include "InvertedIndex.h"
#include <vector>
#include <string>

struct RelativeIndex {
    size_t doc_id;
    float rank;

    // Для тестирования и сравнения
    bool operator==(const RelativeIndex& other) const {
        const float epsilon = 1e-6;
        return (doc_id == other.doc_id) &&
               (std::abs(rank - other.rank) < epsilon);
    }
};

class SearchServer {
public:
    /**
     * @param idx Ссылка на инвертированный индекс
     */
    explicit SearchServer(InvertedIndex& idx) : index(idx) {}

    /**
     * Обрабатывает поисковые запросы
     * @param queries_input Вектор запросов
     * @return Вектор результатов с релевантностью
     */
    std::vector<std::vector<RelativeIndex>> search(
        const std::vector<std::string>& queries_input);

private:
    InvertedIndex& index;

    std::vector<std::pair<size_t, float>> processQuery(
        const std::string& query);

    std::vector<RelativeIndex> calculateRelativeRank(
        const std::vector<std::pair<size_t, float>>& found_docs);
};

#endif // SEARCHSERVER_H
