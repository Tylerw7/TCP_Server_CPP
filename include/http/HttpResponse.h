#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "http/HttpStatus.h"

// *** data.dump() Serializes the JSON object back into a string.


namespace http {


    class HttpResponse {
    public:
        HttpStatus status;
        
        std::unordered_map<std::string, std::string> headers;
        std::string body;

        void set_json(const nlohmann::json& data) {
            headers["Content-Type"] = "application/json";
            body = data.dump();
        }

        static HttpResponse json(HttpStatus status, const nlohmann::json& data) {
            HttpResponse response;
            response.status = status;
            response.set_json(data);
            return response;
        }

        static HttpResponse error(HttpStatus status, const std::string& message) {
            HttpResponse response;
            response.status = status;
            response.set_json({{"error", message }});
            return response;
        }
};

}