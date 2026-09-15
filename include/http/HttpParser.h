#pragma once

#include <string>
#include <unordered_map>
#include "http/HttpRequest.h"

namespace http {

class HttpParser {
public:
    bool parse(const std::string& raw_request, HttpRequest& request);
};                    // <-- class ends HERE

// free function — after the class, still inside namespace http
void parse_query_string(
    const std::string& query,
    std::unordered_map<std::string, std::string>& params
);

}  // namespace http