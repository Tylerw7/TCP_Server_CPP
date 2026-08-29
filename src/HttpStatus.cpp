#include "HttpStatus.h"
#include <string>


std::string status_text(HttpStatus status) {
    switch (status) {
        case HttpStatus::OK:
            return "OK";

        case HttpStatus::Created:
            return "Created";

        case HttpStatus::BadRequest:
            return "Bad Request";

        case HttpStatus::Unauthorized:
            return "Unauthorized";

        case HttpStatus::Forbidden:
            return "Forbidden";

        case HttpStatus::NotFound:
            return "Not Found";

        case HttpStatus::MethodNotAllowed:
            return "Method Not Allowed";

        case HttpStatus::InternalServerError:
            return "Internal Server Error";
    
    }

    return "Unknown";
}