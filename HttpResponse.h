#pragma once

#include <string>
#include <unordered_map>


class HttpResponse {
    public:
        int status_code;
        std::string status_text;
        std::unordered_map<std::string, std::string> headers;
        std::string body;
};