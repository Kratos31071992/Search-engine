ifndef INVERTEDINDEX_H
#define INVERTEDINDEX_H

#include <vector>
#include <string>
#include <map>
#include <mutex>

struct Entry {
    size_t doc_id;
    size_t count;

    // Для тестирования и сравнения
    bool operator==(const Entry& other) const {
        return (doc_id == other.doc_id && count == other.count);
    }
};

class InvertedIndex {
public:
    InvertedIndex() = default;

    /**
     * Обновляет или заполняет базу документов для поиска
     * @param input_docs Содержимое документов
     * @throws std::runtime_error При пустом входном списке
     */
    void UpdateDocumentBase(std::vector<std::string> input_docs);

    /**
     * Возвращает количество вхождений слова в документах
     * @param word Искомое слово
     * @return Вектор пар {doc_id, count}
     */
    std::vector<Entry> GetWordCount(const std::string& word) const;

private:
    void indexDocument(size_t doc_id, const std::string& content);

    std::vector<std::string> docs; // Хранилище документов
    std::map<std::string, std::vector<Entry>> freq_dict; // Частотный словарь
    mutable std::mutex dict_mutex; // Для потокобезопасности
};

#endif // INVERTEDINDEX_H
