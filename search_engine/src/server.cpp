#include "search_server.h"

#include <sstream>
#include <set>
#include <map>
#include <algorithm>
#include <cmath>

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string>& queries_input) {
    std::vector<std::vector<RelativeIndex>> results;

    for (const auto& q : queries_input) {
        std::stringstream ss(q);
        std::set<std::string> unique_words;
        std::string w;
        while (ss >> w) {
            unique_words.insert(w);
        }

        if (unique_words.empty()) {
            results.emplace_back();
            continue;
        }

        std::vector<std::string> words(unique_words.begin(), unique_words.end());

        // Сортировка по частоте (возрастание)
        std::sort(words.begin(), words.end(), [this](const std::string& a, const std::string& b) {
            size_t freq_a = 0, freq_b = 0;
            for (const auto& e : index.GetWordCount(a)) freq_a += e.count;
            for (const auto& e : index.GetWordCount(b)) freq_b += e.count;
            return freq_a < freq_b;
        });

        // Найти документы, содержащие все слова (AND семантика)
        std::map<size_t, size_t> doc_counts;
        std::vector<size_t> candidate_docs;
        for (const auto& word : words) {
            auto counts = index.GetWordCount(word);
            if (counts.empty()) {
                candidate_docs.clear();
                break;
            }
            if (candidate_docs.empty()) {
                for (const auto& entry : counts) {
                    candidate_docs.push_back(entry.doc_id);
                }
            } else {
                std::vector<size_t> new_candidates;
                for (const auto& doc : candidate_docs) {
                    if (std::any_of(counts.begin(), counts.end(), [doc](const Entry& e) { return e.doc_id == doc; })) {
                        new_candidates.push_back(doc);
                    }
                }
                candidate_docs = new_candidates;
                if (candidate_docs.empty()) break;
            }
        }

        if (candidate_docs.empty()) {
            results.push_back({});
            continue;
        }

        // Рассчитать релевантность
        std::map<size_t, float> doc_abs;
        float max_abs = 0.0f;

        for (const auto& doc : candidate_docs) {
            float abs_r = 0.0f;
            for (const auto& word : words) {
                auto counts = index.GetWordCount(word);
                auto it = std::find_if(counts.begin(), counts.end(), [doc](const Entry& e) { return e.doc_id == doc; });
                if (it != counts.end()) abs_r += static_cast<float>(it->count);
            }
            doc_abs[doc] = abs_r;
            if (abs_r > max_abs) max_abs = abs_r;
        }

        std::vector<RelativeIndex> rels;
        for (const auto& [doc, abs_r] : doc_abs) {
            float rank = (max_abs > 0.0f) ? abs_r / max_abs : 0.0f;
            if (rank > 0.0f) rels.push_back({doc, rank});
        }

        // Сортировка по rank desc, doc_id asc
        std::sort(rels.begin(), rels.end(), [](const RelativeIndex& a, const RelativeIndex& b) {
            if (std::fabs(a.rank - b.rank) > 1e-6) return a.rank > b.rank;
            return a.doc_id < b.doc_id;
        });

        // Ограничение max_responses (5 по умолчанию, но в main ограничиваем)
        results.push_back(rels);
    }

    return results;
}
