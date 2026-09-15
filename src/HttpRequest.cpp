#include "http/HttpRequest.h"
#include "http/HttpParser.h"

namespace http {

void HttpRequest::parse_form_body() {
    auto content_type = headers.find("Content-Type");

    if (content_type != headers.end() &&
        content_type->second.find("application/x-www-form-urlencoded")
            != std::string::npos) {

        parse_query_string(body, form_params);
    }
}

}  // namespace http