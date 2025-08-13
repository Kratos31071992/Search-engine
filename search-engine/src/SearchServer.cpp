#include "SearchServer.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>
#include <set>

std::vector<std::vector<RelativeIndex>> SearchServer::search(
    const std::vector<std::string>& queries_input)
{
    std::vector<std::vector<RelativeIndex>> results;
    results.reserve(queries_input.size());

    for (const auto& query : queries_input) {
        if (query.empty()) {
            results.push_back({});
            continue;
        }

        auto found_docs = processQuery(query);
        auto relative_ranks = calculateRelativeRank(found_docs);

        // Сортируем по убыванию релевантности
        std::sort(relative_ranks.begin(), relative_ranks.end(),
            [](const RelativeIndex& a, const RelativeIndex& b) {
                if (a.rank != b.rank) {
                    return a.rank > b.rank;
                }
                return a.doc_id < b.doc_id; // При одинаковой релевантности по возрастанию doc_id
            });

        results.push_back(std::move(relative_ranks));
    }

    return results;
}

std::vector<std::pair<size_t, float>> SearchServer::processQuery(
    const std::string& query)
{
    std::map<size_t, float> doc_abs_rank; // doc_id -> суммарная релевантность
    std::set<std::string> unique_words;
    std::vector<std::vector<Entry>> word_entries;

    // Разбиваем запрос на уникальные слова
    std::stringstream ss(query);
    std::string word;
    while (ss >> word) {
        std::transform(word.begin(), word.end(), word.begin(),
            [](unsigned char c){ return std::tolower(c); });
        if (unique_words.insert(word).second) { // Если слово уникальное
            auto entries = index.GetWordCount(word);
            if (entries.empty()) {
                return {}; // Если хотя бы одно слово не найдено
            }
            word_entries.push_back(std::move(entries));
        }
    }

    if (word_entries.empty()) return {};

    // Сортируем слова по возрастанию частоты (от самых редких)
    std::sort(word_entries.begin(), word_entries.end(),
        [](const std::vector<Entry>& a, const std::vector<Entry>& b) {
            return a.size() < b.size();
        });

    // Находим общие документы, содержащие все слова
    std::set<size_t> common_docs;
    for (const auto& entry : word_entries[0]) {
        common_docs.insert(entry.doc_id);
    }

    for (size_t i = 1; i < word_entries.size() && !common_docs.empty(); ++i) {
        std::set<size_t> temp;
        for (const auto& entry : word_entries[i]) {
            if (common_docs.count(entry.doc_id)) {
                temp.insert(entry.doc_id);
            }
        }
        common_docs = std::move(temp);
    }

    // Рассчитываем абсолютную релевантность
    for (size_t doc_id : common_docs) {
        float rank = 0;
        for (const auto& entries : word_entries) {
            for (const auto& entry : entries) {
                if (entry.doc_id == doc_id) {
                    rank += entry.count;
                    break;
                }
            }
        }
        doc_abs_rank[doc_id] = rank;
    }

    // Преобразуем в вектор пар
    std::vector<std::pair<size_t, float>> result;
    result.reserve(doc_abs_rank.size());
    for (const auto& [doc_id, rank] : doc_abs_rank) {
        result.emplace_back(doc_id, rank);
    }

    return result;
}

std::vector<RelativeIndex> SearchServer::calculateRelativeRank(
    const std::vector<std::pair<size_t, float>>& found_docs)
{
    if (found_docs.empty()) {
        return {};
    }

    // Находим максимальную абсолютную релевантность
    float max_rank = std::max_element(found_docs.begin(), found_docs.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        })->second;

    if (max_rank == 0) {
        return {};
    }

    // Рассчитываем относительную релевантность
    std::vector<RelativeIndex> result;
    result.reserve(found_docs.size());
    for (const auto& [doc_id, rank] : found_docs) {
        result.push_back({doc_id, rank / max_rank});
    }

    return result;
}
