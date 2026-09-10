#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "HttpStatus.h"

// *** data.dump() Serializes the JSON object back into a string.


class HttpResponse {
    public:
        HttpStatus status;
        
        std::unordered_map<std::string, std::string> headers;
        std::string body;

        void set_json(const nlohmann::json& data) {
            headers["Content-Type"] = "application/json";
            body = data.dump();
        }
};