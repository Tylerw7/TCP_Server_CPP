#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "http/HttpParser.h"

namespace http {


  class HttpRequest {

    public:
      std::string method;
      std::string path;
      std::string version;

      std::unordered_map<std::string, std::string> headers;
      std::unordered_map<std::string, std::string> query_params;
      std::unordered_map<std::string, std::string> path_params;
      std::unordered_map<std::string, std::string> form_params;

      std::string body;

      void parse_form_body();

      void parse_query_string(
          const std::string& query,
          std::unordered_map<std::string, std::string>& params
      );

      // Parse the body as JSON. Throws nlohmann::json::parse_error on bad input.
      nlohmann::json json() const {
        return nlohmann::json::parse(body);
      }
};
}

