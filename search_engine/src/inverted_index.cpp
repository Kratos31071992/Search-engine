#include "../include/inverted_index.h"

#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>

std::mutex mtx;

void InvertedIndex::UpdateDocumentBase(std::vector<std::string> input_docs) {
    docs = std::move(input_docs);
    freq_dictionary.clear();

    std::vector<std::thread> threads;
    for (size_t doc_id = 0; doc_id < docs.size(); ++doc_id) {
        threads.emplace_back([this, doc_id]() {
            std::stringstream ss(docs[doc_id]);
            std::string word;
            std::map<std::string, size_t> local_freq;
            while (ss >> word) {
                ++local_freq[word];
            }
            {
                std::lock_guard<std::mutex> lock(mtx);
                for (const auto& [w, c] : local_freq) {
                    freq_dictionary[w].push_back({doc_id, c});
                }
            }
        });
    }

    for (auto& t : threads) t.join();

    for (auto& [w, entries] : freq_dictionary) {
        std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
            return a.doc_id < b.doc_id;
        });
    }
}

std::vector<Entry> InvertedIndex::GetWordCount(const std::string& word) {
    auto it = freq_dictionary.find(word);
    if (it != freq_dictionary.end()) {
        return it->second;
    }
    return {};
}