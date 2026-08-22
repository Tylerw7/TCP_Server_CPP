#include "HttpParser.h"

#include <sstream>

HttpRequest HttpParser::parse(const std::string& raw_request) {

    HttpRequest request;

    // -------------------------
    // Parse request line
    // -------------------------

    size_t end_of_line =
        raw_request.find("\r\n");

    if (end_of_line == std::string::npos) {
        return request;
    }

    std::string request_line =
        raw_request.substr(0, end_of_line);

    std::istringstream request_stream(request_line);

    request_stream
        >> request.method
        >> request.path
        >> request.version;


    // -------------------------
    // Parse headers
    // -------------------------

    size_t header_start =
        end_of_line + 2;

    while (true) {

        size_t header_end =
            raw_request.find("\r\n", header_start);

        if (header_end == std::string::npos) {
            break;
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

        if (colon != std::string::npos) {

            std::string name =
                header_line.substr(0, colon);

            std::string value =
                header_line.substr(colon + 1);

            if (!value.empty() && value[0] == ' ') {
                value.erase(0, 1);
            }

            request.headers[name] = value;
        }

        header_start = header_end + 2;
    }

    return request;
}