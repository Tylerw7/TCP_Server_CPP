#include "HttpParser.h"

#include <algorithm>
#include <sstream>

bool is_valid_method(std::string& method) {
    return method == "GET"
        || method == "POST"
        || method == "PUT"
        || method == "PATCH"
        || method == "DELETE"
        || method == "HEAD"
        || method == "OPTIONS";

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


    return true;
}