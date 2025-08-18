#include "../include/converter_json.h"

#include <nlohmann/json.hpp>
using nlohmann::json;

#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>

std::vector<std::string> ConverterJSON::GetTextDocuments() {
    std::ifstream config_file("config.json");
    if (!config_file) {
        std::cerr << "config file is missing" << std::endl;
        throw std::runtime_error("config file is missing");
    }

    json config;
    try {
        config = json::parse(config_file);
    } catch (...) {
        std::cerr << "config file is empty" << std::endl;
        throw std::runtime_error("config file is empty");
    }

    if (!config.contains("config")) {
        std::cerr << "config file is empty" << std::endl;
        throw std::runtime_error("config file is empty");
    }

    auto conf = config["config"];

    if (!conf.contains("version") || conf["version"] != "0.1") {
        std::cerr << "config.json has incorrect file version" << std::endl;
        throw std::runtime_error("config.json has incorrect file version");
    }

    if (conf.contains("name")) {
        std::cout << "Starting " << conf["name"] << std::endl;
    }

    if (!config.contains("files")) {
        throw std::runtime_error("no files in config");
    }

    std::vector<std::string> texts;
    for (const auto& file : config["files"]) {
        std::string path = file.get<std::string>();
        std::ifstream in(path);
        if (!in) {
            std::cout << "File " << path << " not found" << std::endl;
            continue;
        }
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        texts.push_back(content);
    }
    return texts;
}

int ConverterJSON::GetResponsesLimit() {
    std::ifstream config_file("config.json");
    json config = json::parse(config_file);
    auto conf = config["config"];
    return conf.contains("max_responses") ? conf["max_responses"].get<int>() : 5;
}

std::vector<std::string> ConverterJSON::GetRequests() {
    std::ifstream req_file("requests.json");
    if (!req_file) {
        throw std::runtime_error("requests file is missing");
    }
    json reqj = json::parse(req_file);
    if (!reqj.contains("requests")) {
        throw std::runtime_error("no requests");
    }
    std::vector<std::string> requests;
    for (const auto& r : reqj["requests"]) {
        requests.push_back(r.get<std::string>());
    }
    return requests;
}

void ConverterJSON::putAnswers(std::vector<std::vector<std::pair<int, float>>> answers) {
    json ansj = json::object();
    ansj["answers"] = json::object();

    size_t req_num = 1;
    for (auto& res : answers) {
        std::string req_id = "request" + std::string(3 - std::to_string(req_num).length(), '0') + std::to_string(req_num);
        json req_ans = json::object();
        if (res.empty()) {
            req_ans["result"] = "false";
        } else {
            req_ans["result"] = "true";
            if (res.size() == 1) {
                req_ans["docid"] = res.front().first;
                req_ans["rank"] = res.front().second;
            } else {
                json rel_arr = json::array();
                for (auto& p : res) {
                    json obj = json::object();
                    obj["docid"] = p.first;
                    obj["rank"] = p.second;
                    rel_arr.push_back(obj);
                }
                req_ans["relevance"] = rel_arr;
            }
        }
        ansj["answers"][req_id] = req_ans;
        ++req_num;
    }

    std::ofstream out("answers.json", std::ios::trunc);
    if (out) {
        out << std::setw(4) << ansj << std::endl;
    } else {
        throw std::runtime_error("cannot write answers.json");
    }
}