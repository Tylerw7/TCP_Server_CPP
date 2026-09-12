#include "http/HttpParser.h"

#include <algorithm>
#include <sstream>

namespace http {

bool is_valid_method(std::string& method) {
    return method == "GET"
        || method == "POST"
        || method == "PUT"
        || method == "PATCH"
        || method == "DELETE"
        || method == "HEAD"
        || method == "OPTIONS";

}

// Pair parser helper
void parse_query_string(
    const std::string& query,
    std::unordered_map<std::string, std::string>& params
) {
    size_t start = 0;

    while (start < query.size()) {

        // Find the end of this pair (next '&' or end of string).
        size_t amp = query.find('&', start);
        if (amp == std::string::npos) {
            amp = query.size();
        }

        std::string pair = query.substr(start, amp - start);

        // Split the pair on '='.
        size_t equals = pair.find('=');

        if (equals != std::string::npos) {
            std::string key = pair.substr(0, equals);
            std::string value = pair.substr(equals + 1);
            params[key] = value;
        } else if (!pair.empty()) {
            // A key with no '=' (e.g. "?debug") — store with empty value.
            params[pair] = "";
        }

        start = amp + 1;
    }
}

bool HttpParser::parse(
    const std::string& raw_request,
    HttpRequest& request
) {

    // ---------------------------------------------------------
    // Find request line
    // ---------------------------------------------------------

    size_t end_of_line =
        raw_request.find("\r\n");

    if (end_of_line == std::string::npos) {
        return false;
    }


    // ---------------------------------------------------------
    // Parse request line
    // ---------------------------------------------------------

    std::string request_line =
        raw_request.substr(
            0,
            end_of_line
        );

    std::istringstream request_stream(
        request_line
    );


    if (!(request_stream
        >> request.method
        >> request.path
        >> request.version)) {

        return false;
    }

    if (!is_valid_method(request.method)) return false;


    // ---------------------------------------------------------
    // Make sure there isn't extra garbage
    // ---------------------------------------------------------

    std::string extra;

    if (request_stream >> extra) {
        return false;
    }


    // ---------------------------------------------------------
    // Validate HTTP version
    // ---------------------------------------------------------

    if (request.version != "HTTP/1.1") {
        return false;
    }

    // ---------------------------------------------------------
    // Split query string off the path
    // ---------------------------------------------------------
    size_t question = request.path.find('?');

    if (question != std::string::npos) {
        std::string query = request.path.substr(question + 1);
        request.path = request.path.substr(0, question);
        parse_query_string(query, request.query_params);
    }



    // ---------------------------------------------------------
    // Parse headers
    // ---------------------------------------------------------

    size_t header_start =
        end_of_line + 2;


    while (true) {

        size_t header_end =
            raw_request.find(
                "\r\n",
                header_start
            );


        if (header_end == std::string::npos) {
            return false;
        }


        // Empty line = end of headers
        if (header_end == header_start) {
            break;
        }


        std::string header_line =
            raw_request.substr(
                header_start,
                header_end - header_start
            );


        size_t colon =
            header_line.find(':');


        if (colon == std::string::npos) {
            return false;
        }


        std::string name =
            header_line.substr(
                0,
                colon
            );


        std::string value =
            header_line.substr(
                colon + 1
            );


        // Remove leading whitespace
        if (!value.empty() &&
            value[0] == ' ') {

            value.erase(0, 1);
        }


        request.headers[name] = value;


        header_start =
            header_end + 2;
    }

    // parse body
    size_t body_start = header_start + 2;

    if (body_start < raw_request.size()) {
        request.body = raw_request.substr(body_start);
    }


    return true;
}

}