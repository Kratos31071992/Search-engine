#include "../include/converter_json.h"
#include "../include/inverted_index.h"
#include "../include/search_server.h"

int main() {
    try {
        ConverterJSON conv;
        auto docs = conv.GetTextDocuments();
        InvertedIndex idx;
        idx.UpdateDocumentBase(docs);
        SearchServer srv(idx);
        auto requests = conv.GetRequests();
        auto results = srv.search(requests);
        int limit = conv.GetResponsesLimit();
        std::vector<std::vector<std::pair<int, float>>> answers;
        for (auto& res : results) {
            std::vector<std::pair<int, float>> v;
            for (const auto& ri : res) {
                v.emplace_back(static_cast<int>(ri.doc_id), ri.rank);
            }
            if (v.size() > static_cast<size_t>(limit)) {
                v.resize(limit);
            }
            answers.push_back(v);
        }
        conv.putAnswers(answers);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}