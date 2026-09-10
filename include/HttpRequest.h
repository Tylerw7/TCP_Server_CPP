#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

class HttpRequest {

    public:
      std::string method;
      std::string path;
      std::string version;

      std::unordered_map<std::string, std::string> headers;

      std::string body;

      // Parse the body as JSON. Throws nlohmann::json::parse_error on bad input.
      nlohmann::json json() const {
        return nlohmann::json::parse(body);
      }
};