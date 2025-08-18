#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <vector>
#include <map>
#include <string>

struct Entry {
    size_t doc_id, count;
    bool operator==(const Entry& other) const {
        return (doc_id == other.doc_id &&
                count == other.count);
    }
};

class InvertedIndex {
public:
    InvertedIndex() = default;

    void UpdateDocumentBase(std::vector<std::string> input_docs);

    std::vector<Entry> GetWordCount(const std::string& word);

private:
    std::vector<std::string> docs; // список содержимого документов
    std::map<std::string, std::vector<Entry>> freq_dictionary; // частотный словарь
};

#endif // INVERTED_INDEX_H/CLIONPROJECTS_INVERTED_INDEX_H