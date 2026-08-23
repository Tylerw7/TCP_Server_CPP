#include "HttpResponseBuilder.h"

#include <sstream>

std::string HttpResponseBuilder::build(
    const HttpResponse& response
) {
    std::ostringstream output;

    // Status line
    output
        << "HTTP/1.1 "
        << response.status_code
        << " "
        << response.status_text
        << "\r\n";

    // Headers
    for (const auto& header : response.headers) {
        output
            << header.first
            << ": "
            << header.second
            << "\r\n";
    }

    // End of headers
    output << "\r\n";

    // Body
    output << response.body;

    return output.str();
}