#ifndef SEARCH_SERVER_H
#define SEARCH_SERVER_H

#include "inverted_index.h"

struct RelativeIndex {
    size_t doc_id;
    float rank;
    bool operator==(const RelativeIndex& other) const {
        return (doc_id == other.doc_id && rank == other.rank);
    }
};

class SearchServer {
public:
    explicit SearchServer(InvertedIndex& idx, size_t responses_limit = 5)
        : index(idx), responses_limit(responses_limit) { };

    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string>& queries_input);

private:
    InvertedIndex index;
    size_t responses_limit;
};

#endif // SEARCH_SERVER_H