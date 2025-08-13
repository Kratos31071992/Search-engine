#include "ConverterJSON.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

std::vector<std::string> ConverterJSON::GetTextDocuments() {
    json config;
    try {
        config = readConfigFile();
    } catch (const std::exception& e) {
        std::cerr << "Config error: " << e.what() << std::endl;
        return {};
    }

    std::vector<std::string> documents;
    if (!config.contains("files")) {
        std::cerr << "Warning: No 'files' section in config.json" << std::endl;
        return documents;
    }

    for (const auto& filePath : config["files"]) {
        try {
            std::ifstream file(filePath.get<std::string>());
            if (!file.is_open()) {
                std::cerr << "File not found: " << filePath << std::endl;
                continue;
            }
            std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
            documents.push_back(content);
        } catch (const std::exception& e) {
            std::cerr << "Error reading file " << filePath << ": " << e.what() << std::endl;
        }
    }
    return documents;
}

int ConverterJSON::GetResponsesLimit() const {
    try {
        json config = readConfigFile();
        if (config.contains("config") && config["config"].contains("max_responses")) {
            return config["config"]["max_responses"].get<int>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading config: " << e.what() << std::endl;
    }
    return 5;
}

std::vector<std::string> ConverterJSON::GetRequests() {
    std::ifstream requestsFile(requestsPath);
    if (!requestsFile.is_open()) {
        throw std::runtime_error("Failed to open requests.json");
    }

    json requestsData;
    try {
        requestsFile >> requestsData;
    } catch (const json::exception& e) {
        throw std::runtime_error("Invalid JSON in requests file: " + std::string(e.what()));
    }

    std::vector<std::string> requests;
    if (requestsData.contains("requests")) {
        for (const auto& request : requestsData["requests"]) {
            requests.push_back(request.get<std::string>());
        }
    }
    return requests;
}

void ConverterJSON::putAnswers(const std::vector<std::vector<std::pair<int, float>>>& answers) {
    json result;

    for (size_t i = 0; i < answers.size(); ++i) {
        std::string requestId = "request" + std::to_string(i + 1);
        if (i + 1 < 10) requestId.insert(7, "00");
        else if (i + 1 < 100) requestId.insert(7, "0");

        if (answers[i].empty()) {
            result["answers"][requestId]["result"] = false;
        } else {
            result["answers"][requestId]["result"] = true;
            for (const auto& [docId, rank] : answers[i]) {
                result["answers"][requestId]["relevance"].push_back({
                    {"docid", docId},
                    {"rank", rank}
                });
            }
        }
    }

    std::ofstream answersFile(answersPath);
    if (!answersFile.is_open()) {
        throw std::runtime_error("Failed to create answers.json");
    }
    answersFile << result.dump(4);
}

json ConverterJSON::readConfigFile() const {
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        throw std::runtime_error("Config file not found");
    }

    json config;
    try {
        configFile >> config;
    } catch (const json::exception& e) {
        throw std::runtime_error("Invalid JSON in config file: " + std::string(e.what()));
    }

    if (!config.contains("config")) {
        throw std::runtime_error("Missing 'config' section");
    }

    if (config["config"].contains("version") &&
        config["config"]["version"].get<std::string>() != "0.1") {
        throw std::runtime_error("Unsupported config version");
    }

    return config;
}
