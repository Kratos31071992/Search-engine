#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H

#include <vector>
#include <string>
#include <utility>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class ConverterJSON {
public:
    ConverterJSON() = default;

    std::vector<std::string> GetTextDocuments();
    int GetResponsesLimit() const;
    std::vector<std::string> GetRequests();
    void putAnswers(const std::vector<std::vector<std::pair<int, float>>>& answers);

private:
    json readConfigFile() const;

    const std::string configPath = "config.json";
    const std::string requestsPath = "requests.json";
    const std::string answersPath = "answers.json";
};

#endif // CONVERTERJS
