#pragma once

// DashAPI — single-include convenience header.
// Include this to get the whole framework.

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/HttpStatus.h"
#include "http/Router.h"
#include "http/HttpServer.h"
#include "http/HttpsServer.h"

// Expose the framework's types under the dashapi namespace,
// so users write dashapi::Router, dashapi::HttpServer, etc.
namespace dashapi {
    using http::HttpRequest;
    using http::HttpResponse;
    using http::HttpStatus;
    using http::Router;
    using http::HttpServer;
    using http::HttpsServer;
    using http::status_text;
}