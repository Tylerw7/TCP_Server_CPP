#include "HttpResponseBuilder.h"

#include <sstream>


std::string HttpResponseBuilder::build(
    const HttpResponse& response
) {

    std::ostringstream output;


    // ---------------------------------------------------------
    // STATUS LINE
    // ---------------------------------------------------------

    output
        << "HTTP/1.1 "
        << static_cast<int>(response.status)
        << " "
        << status_text(response.status)
        << "\r\n";


    // ---------------------------------------------------------
    // HEADERS
    // ---------------------------------------------------------

    for (const auto& header :
         response.headers) {

        output
            << header.first
            << ": "
            << header.second
            << "\r\n";
    }


    // ---------------------------------------------------------
    // CONTENT LENGTH
    // ---------------------------------------------------------

    output
        << "Content-Length: "
        << response.body.size()
        << "\r\n";


    // ---------------------------------------------------------
    // END OF HEADERS
    // ---------------------------------------------------------

    output << "\r\n";


    // ---------------------------------------------------------
    // BODY
    // ---------------------------------------------------------

    output << response.body;


    return output.str();
}