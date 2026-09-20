<p align="center">
  <img src="dashapi_logo_preview.png" alt="DashAPI" width="500">
</p>

# DashAPI

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat&logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C?style=flat&logo=cmake&logoColor=white)
![OpenSSL](https://img.shields.io/badge/OpenSSL-3.x-721412?style=flat&logo=openssl&logoColor=white)
![nlohmann/json](https://img.shields.io/badge/nlohmann%2Fjson-3.12-blue?style=flat)
![Catch2](https://img.shields.io/badge/Catch2-3.5-red?style=flat)
![CTest](https://img.shields.io/badge/tests-CTest-brightgreen?style=flat)
![License](https://img.shields.io/badge/license-MIT-green?style=flat)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey?style=flat)

**DashAPI** is a lightweight, from-scratch HTTP server framework for C++17. It gives you an
expressive, Express-style routing API on top of raw POSIX sockets — routes, middleware,
path and query parameters, JSON, per-connection threading, keep-alive, and optional TLS —
with no heavyweight dependencies and a small, readable codebase you can actually understand
end to end.

It is designed to be embedded in your own project: define your routes in your own `main()`,
link against the library, and ship a real HTTP or HTTPS service. Everything is available
through a single include — `#include <dashapi/dashapi.h>` — under the `dashapi` namespace.

---

## Table of Contents

- [Why DashAPI](#why-dashapi)
- [Features](#features)
- [Architecture Overview](#architecture-overview)
- [Requirements](#requirements)
- [Installation & Build](#installation--build)
- [Quick Start](#quick-start)
- [Core Concepts](#core-concepts)
  - [The Include & Namespace](#the-include--namespace)
  - [The Router](#the-router)
  - [Requests](#requests)
  - [Responses](#responses)
  - [Status Codes](#status-codes)
- [Routing](#routing)
  - [HTTP Methods](#http-methods)
  - [Path Parameters](#path-parameters)
  - [Query Parameters](#query-parameters)
- [Request Bodies](#request-bodies)
  - [JSON Bodies](#json-bodies)
  - [Form-Encoded Bodies](#form-encoded-bodies)
- [JSON Responses](#json-responses)
- [Middleware](#middleware)
- [Error Handling](#error-handling)
- [Concurrency](#concurrency)
- [Keep-Alive](#keep-alive)
- [HTTPS / TLS](#https--tls)
- [Full API Reference](#full-api-reference)
- [Testing](#testing)
- [Project Structure](#project-structure)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)

---

## Why DashAPI

Most C++ web frameworks are either enormous (bringing in Boost.Asio and a large surface area)
or opaque. DashAPI takes the opposite approach: it is a small, legible framework built directly
on the Berkeley sockets API, where every layer — parsing, routing, the response builder, the
threading model — is plain C++ you can read in an afternoon.

That makes it a good fit for:

- **Learning** how HTTP servers actually work, from the TCP layer up.
- **Small services and internal APIs** where you want a single self-contained binary.
- **Embedding** an HTTP interface into an existing C++ application.

It is **not** trying to be a high-throughput production web server competing with nginx or
Drogon. For high-scale public traffic you would typically run DashAPI behind a reverse proxy.

---

## Features

- **Single-include** — `#include <dashapi/dashapi.h>` brings in the whole framework under the `dashapi` namespace.
- **Express-style routing** — `router.get("/path", handler)`, `.post`, `.put`, `.patch`, `.delete_route`.
- **Path parameters** — `/users/:id`, captured into `request.path_params`.
- **Query parameters** — `/search?q=cats`, parsed into `request.query_params`.
- **JSON support** — first-class parsing and serialization via [nlohmann/json](https://github.com/nlohmann/json).
- **Form parsing** — `application/x-www-form-urlencoded` bodies parsed into `request.form_params`.
- **Middleware** — Express-style `(request, next)` chain for logging, auth, headers, and more.
- **Centralized error handling** — a throwing handler becomes a clean `500` instead of crashing.
- **Per-connection threading** — every connection is served on its own thread.
- **HTTP keep-alive** — connections are reused across requests (HTTP/1.1 default).
- **TLS / HTTPS** — optional encrypted transport via OpenSSL, as a separate `HttpsServer`.
- **Tested** — unit tests with Catch2, orchestrated by CTest.

---

## Architecture Overview

A request flows through the framework like this:

```
  TCP socket
     │
     ▼
  HttpServer::run()            accept() a connection, spawn a thread
     │
     ▼
  HttpServer::handle_client()  read bytes, loop for keep-alive
     │
     ▼
  HttpParser::parse()          raw bytes ──► HttpRequest
     │                         (method, path, version, headers, query params, body)
     ▼
  HttpRequest::parse_form_body()   populate form_params if urlencoded
     │
     ▼
  Router::handle()             middleware chain ──► route match ──► handler
     │                         (path params captured here; errors caught here)
     ▼
  HttpResponse                 returned by the handler
     │
     ▼
  HttpResponseBuilder::build()  HttpResponse ──► raw HTTP response text
     │
     ▼
  send()/SSL_write()           back over the socket
```

The design separates **transport** (`HttpServer` / `HttpsServer`) from **request handling**
(`Router`), so the same routes work identically over plaintext or TLS.

---

## Requirements

| Tool           | Version | Purpose                                  |
|----------------|---------|------------------------------------------|
| C++ compiler   | C++17   | Clang or GCC                             |
| CMake          | 3.20+   | Build system                             |
| OpenSSL        | 3.x     | TLS / HTTPS support                      |
| nlohmann/json  | 3.12    | JSON (fetched automatically by CMake)    |
| Catch2         | 3.5     | Unit tests (fetched automatically)       |

`nlohmann/json` and `Catch2` are pulled in automatically via CMake's `FetchContent` — you do
not need to install them yourself. Only a compiler, CMake, and OpenSSL need to be present on
your system.

### Installing OpenSSL

**macOS (Homebrew):**

```bash
brew install openssl@3
```

**Debian / Ubuntu:**

```bash
sudo apt-get install libssl-dev
```

---

## Installation & Build

Clone the repository and build with CMake:

```bash
git clone https://github.com/yourname/DashAPI.git
cd DashAPI
cmake -B build
cmake --build build
```

### macOS note

Homebrew installs OpenSSL outside the default search path, so pass its location to CMake:

```bash
cmake -B build -DOPENSSL_ROOT_DIR=$(brew --prefix openssl@3)
cmake --build build
```

This is cached, so subsequent `cmake --build build` calls remember it.

After building, the example binaries are in `build/`:

```bash
./build/basic_server     # plaintext HTTP on :8080
./build/secure_server    # HTTPS on :8443 (needs cert.pem/key.pem — see HTTPS section)
```

---

## Quick Start

A minimal server:

```cpp
#include <dashapi/dashapi.h>

using namespace dashapi;

int main() {
    Router router;

    router.get("/", [](const HttpRequest& request) {
        HttpResponse response;
        response.status = HttpStatus::OK;
        response.headers["Content-Type"] = "text/plain";
        response.body = "Hello from DashAPI";
        return response;
    });

    HttpServer server(8080, router);
    server.run();

    return 0;
}
```

```bash
curl http://localhost:8080/
# Hello from DashAPI
```

The key pattern: **build and configure the `Router` completely, then hand it to the server.**
All routes and middleware must be registered before `server.run()` — the server takes ownership
of the router and serves requests concurrently, so nothing should be added to it afterward.

---

## Core Concepts

### The Include & Namespace

Everything in DashAPI is available through one header:

```cpp
#include <dashapi/dashapi.h>
```

This single include pulls in the request and response types, status codes, the router, and both
the HTTP and HTTPS servers. All of the framework's types live in the `dashapi` namespace, so you
can either qualify them (`dashapi::Router`) or bring them into scope with:

```cpp
using namespace dashapi;
```

Every example in this document assumes the single include and `using namespace dashapi;`. The
individual headers (`<http/Router.h>`, etc.) still exist under the hood, but the umbrella header
is the intended entry point.

### The Router

The `Router` holds your routes and middleware and turns an `HttpRequest` into an `HttpResponse`.
You register handlers on it, then pass it to a server:

```cpp
Router router;
router.get("/", handler);
router.post("/submit", handler);

HttpServer server(8080, router);
server.run();
```

A **handler** is any callable matching:

```cpp
std::function<HttpResponse(const HttpRequest&)>
```

Typically a lambda. It receives the incoming request and returns the response to send.

### Requests

`HttpRequest` is what your handler receives. Its fields:

| Field           | Type                                          | Description                                  |
|-----------------|-----------------------------------------------|----------------------------------------------|
| `method`        | `std::string`                                 | `"GET"`, `"POST"`, etc.                       |
| `path`          | `std::string`                                 | Path with the query string stripped off      |
| `version`       | `std::string`                                 | e.g. `"HTTP/1.1"`                             |
| `headers`       | `std::unordered_map<std::string,std::string>` | Request headers                              |
| `query_params`  | `std::unordered_map<std::string,std::string>` | Parsed `?key=value` pairs                    |
| `path_params`   | `std::unordered_map<std::string,std::string>` | Captured `:name` segments                    |
| `form_params`   | `std::unordered_map<std::string,std::string>` | Parsed urlencoded body                       |
| `body`          | `std::string`                                 | Raw request body                             |

Plus a helper:

```cpp
nlohmann::json json() const;   // parse the body as JSON (throws on invalid JSON)
```

### Responses

`HttpResponse` is what your handler returns. Its fields:

| Field     | Type                                          | Description                     |
|-----------|-----------------------------------------------|---------------------------------|
| `status`  | `HttpStatus`                                  | Status code enum                |
| `headers` | `std::unordered_map<std::string,std::string>` | Response headers                |
| `body`    | `std::string`                                 | Response body                   |

Plus helpers for JSON:

```cpp
void set_json(const nlohmann::json& data);
                                    // set body + Content-Type: application/json

static HttpResponse json(HttpStatus status, const nlohmann::json& data);
                                    // build a full JSON response in one call

static HttpResponse error(HttpStatus status, const std::string& message);
                                    // build {"error": "..."} in one call
```

### Status Codes

`HttpStatus` is a scoped enum of common codes:

```cpp
enum class HttpStatus {
    OK                  = 200,
    Created             = 201,
    BadRequest          = 400,
    Unauthorized        = 401,
    Forbidden           = 403,
    NotFound            = 404,
    MethodNotAllowed    = 405,
    InternalServerError = 500,
    NotImplemented      = 501
};
```

`status_text(HttpStatus)` returns the reason phrase (e.g. `HttpStatus::NotFound` → `"Not Found"`).

---

## Routing

### HTTP Methods

Each HTTP method has a registration function on the router:

```cpp
router.get("/items",       handler);
router.post("/items",      handler);
router.put("/items/:id",   handler);
router.patch("/items/:id", handler);
router.delete_route("/items/:id", handler);   // "delete" is a C++ keyword, hence delete_route
```

Routes are matched in **registration order**, and the first match wins. If no route matches,
the router returns a `404 Not Found`.

> **Note:** matching is by method *and* path pattern. Because the first match wins, register
> more specific literal routes before parameterized ones if they could overlap (e.g. register
> `/users/me` before `/users/:id`).

### Path Parameters

Segments beginning with `:` are captured as named parameters:

```cpp
router.get("/users/:id", [](const HttpRequest& request) {
    std::string id = request.path_params.at("id");

    HttpResponse response;
    response.status = HttpStatus::OK;
    response.body = "Requested user: " + id;
    return response;
});
```

```bash
curl http://localhost:8080/users/42
# Requested user: 42
```

Multiple parameters work as expected: `/users/:userId/posts/:postId` captures both `userId`
and `postId`. A pattern only matches a path with the **same number of segments**, so
`/users/:id` will not match `/users/42/extra`.

Since a matched route guarantees the parameter exists, `path_params.at("id")` is safe inside
the handler.

### Query Parameters

The query string is automatically split off the path and parsed:

```cpp
router.get("/search", [](const HttpRequest& request) {
    auto it = request.query_params.find("q");
    std::string term = (it != request.query_params.end()) ? it->second : "(none)";

    HttpResponse response;
    response.status = HttpStatus::OK;
    response.body = "You searched for: " + term;
    return response;
});
```

```bash
curl "http://localhost:8080/search?q=cats&limit=10"
# You searched for: cats
```

Use `.find()` rather than `operator[]` when reading, so a missing key doesn't silently insert
an empty entry.

> **Note:** query and form values are **not** URL-decoded — `%20` and `+` come through
> literally. Decoding is on the roadmap.

---

## Request Bodies

### JSON Bodies

Parse an incoming JSON body with `request.json()`:

```cpp
router.post("/echo", [](const HttpRequest& request) {
    HttpResponse response;

    try {
        nlohmann::json body = request.json();

        std::string name = body.at("name");
        int age = body.at("age");

        nlohmann::json reply;
        reply["message"] = "Name: " + name + " Age: " + std::to_string(age);

        response.status = HttpStatus::OK;
        response.set_json(reply);

    } catch (const nlohmann::json::exception& error) {
        response.status = HttpStatus::BadRequest;
        response.set_json({{"error", "Invalid JSON"}});
    }

    return response;
});
```

```bash
curl -X POST http://localhost:8080/echo \
     -H "Content-Type: application/json" \
     -d '{"name":"Tyler","age":30}'
# {"message":"Name: Tyler Age: 30"}
```

`request.json()` throws `nlohmann::json::parse_error` on malformed input and
`nlohmann::json::exception` (its base) on things like missing keys or type mismatches, so wrap
it in a `try/catch` and return a `400` on failure. Using `.at("key")` (rather than `[]`) makes
missing keys throw, so they're caught the same way.

### Form-Encoded Bodies

When a request arrives with `Content-Type: application/x-www-form-urlencoded`, the body is
parsed into `form_params`:

```cpp
router.post("/submit", [](const HttpRequest& request) {
    auto it = request.form_params.find("name");
    std::string name = (it != request.form_params.end()) ? it->second : "(none)";

    HttpResponse response;
    response.status = HttpStatus::OK;
    response.body = "Form submitted by: " + name;
    return response;
});
```

```bash
curl -X POST http://localhost:8080/submit -d "name=Tyler&age=30"
# Form submitted by: Tyler
```

The Content-Type check is a substring match, so charset suffixes like
`application/x-www-form-urlencoded; charset=utf-8` are handled correctly. Non-form bodies (e.g.
JSON) leave `form_params` empty.

---

## JSON Responses

Two static helpers on `HttpResponse` make JSON APIs concise.

**`HttpResponse::json(status, data)`** builds a complete JSON response:

```cpp
router.get("/api/status", [](const HttpRequest& request) {
    nlohmann::json data;
    data["service"] = "DashAPI";
    data["status"]  = "ok";
    data["version"] = "1.0";

    return HttpResponse::json(HttpStatus::OK, data);
});
```

```bash
curl http://localhost:8080/api/status
# {"service":"DashAPI","status":"ok","version":"1.0"}
```

**`HttpResponse::error(status, message)`** builds a standard `{"error": "..."}` body:

```cpp
router.get("/api/users/:id", [](const HttpRequest& request) {
    std::string id = request.path_params.at("id");

    if (id != "42") {
        return HttpResponse::error(HttpStatus::NotFound, "user not found");
    }

    nlohmann::json user;
    user["id"]   = id;
    user["name"] = "Tyler";
    return HttpResponse::json(HttpStatus::OK, user);
});
```

Both set `Content-Type: application/json` automatically and give your API a consistent shape.

---

## Middleware

Middleware runs *around* your route handlers — ideal for logging, authentication, and adding
headers. A middleware receives the request and a `next` callable; it decides whether to call
`next` (to continue down the chain to the route) or to short-circuit and return early.

```cpp
using Middleware = std::function<HttpResponse(
    const HttpRequest&,
    std::function<HttpResponse(const HttpRequest&)>   // next
)>;
```

Register middleware with `router.use(...)`:

```cpp
// Logging middleware
router.use([](const HttpRequest& request,
              std::function<HttpResponse(const HttpRequest&)> next) {
    std::cout << "-> " << request.method << " " << request.path << '\n';
    HttpResponse response = next(request);
    std::cout << "<- " << static_cast<int>(response.status) << '\n';
    return response;
});
```

### Execution order

Middleware runs **outermost-first, in registration order**. Think of it as nested shells around
the route — the request travels inward through each middleware to reach the handler, and the
response travels back outward:

```
request →  [A  [B  [ ROUTE ]  B]  A]  → response
```

If middleware `A` is registered before `B`, then for each request: `A` runs, calls `next`,
which runs `B`, which calls `next`, which runs the route; then `B` finishes, then `A` finishes.

### Short-circuiting

A middleware that does **not** call `next` stops the chain — the route never runs. This is how
authentication is enforced:

```cpp
router.use([](const HttpRequest& request,
              std::function<HttpResponse(const HttpRequest&)> next) {

    auto it = request.headers.find("Authorization");
    if (it == request.headers.end()) {
        return HttpResponse::error(HttpStatus::Unauthorized, "missing token");
    }

    return next(request);   // authorized — continue
});
```

---

## Error Handling

If a route handler (or any middleware) throws an exception, DashAPI catches it centrally and
returns a `500 Internal Server Error` instead of crashing the connection or the server. This is
handled inside `Router::handle()`, which wraps the entire middleware-and-route chain in a
`try/catch`.

```cpp
router.get("/boom", [](const HttpRequest&) -> HttpResponse {
    throw std::runtime_error("something failed");
});
```

```bash
curl http://localhost:8080/boom
# 500 - Internal Server Error
```

Both `std::exception`-derived and unknown (`catch (...)`) exceptions are handled. This means an
unexpected failure deep inside a handler degrades gracefully to a `500` rather than taking down
the process.

---

## Concurrency

Each accepted connection is handled on its own detached `std::thread`, so multiple clients are
served simultaneously — a slow request does not block others.

Because all threads share the single `Router`, the framework relies on one rule for
thread-safety: **the router is fully configured before `run()` and never mutated afterward.**
Once serving begins, `Router::handle()` only *reads* the routes and middleware (all per-request
mutable state is local to the calling thread), so concurrent access is safe without locks.

This is why the API is shaped the way it is — you build the router in `main()`, then hand it off.

> **Logging caveat:** because threads share `std::cout`, log lines from concurrent requests can
> interleave. This is cosmetic and does not affect correctness.

> **Shared state:** DashAPI's router is safe to share across threads, but any of *your own*
> mutable state touched inside handlers (a cache, an in-memory store) is not automatically —
> guard it with a `std::mutex` if concurrent requests can modify it.

---

## Keep-Alive

DashAPI supports HTTP/1.1 persistent connections. After sending a response, the server keeps
the connection open and loops to read the next request on the same socket, avoiding a fresh TCP
handshake per request.

The decision follows standard rules:

- `Connection: close` header → the connection is closed after the response.
- `Connection: keep-alive` header → the connection is kept open.
- No header → defaults to keep-alive for `HTTP/1.1`.

The server echoes its decision back in the response's `Connection` header.

```bash
curl -v http://localhost:8080/ http://localhost:8080/api/status
# ... look for "Re-using existing connection" before the second request
```

> **Limitation:** HTTP pipelining (a client sending multiple requests before reading responses)
> is not fully handled — the common request/response/request pattern used by browsers and curl
> works correctly.

---

## HTTPS / TLS

TLS is provided by a separate `HttpsServer` class that reuses all of your existing routing. Your
handlers and router are identical; only the transport changes.

### 1. Generate a development certificate

For local testing, create a self-signed certificate and key:

```bash
openssl req -x509 -newkey rsa:2048 -nodes \
  -keyout key.pem -out cert.pem -days 365 \
  -subj "/CN=localhost"
```

This produces `cert.pem` (the public certificate) and `key.pem` (the private key). **Never
commit these to version control** — add them to `.gitignore`. In production, users supply their
own certificate (for example from Let's Encrypt).

### 2. Run an HTTPS server

```cpp
#include <dashapi/dashapi.h>

using namespace dashapi;

int main() {
    Router router;

    router.get("/", [](const HttpRequest& request) {
        HttpResponse response;
        response.status = HttpStatus::OK;
        response.headers["Content-Type"] = "text/plain";
        response.body = "Hello over HTTPS!";
        return response;
    });

    HttpsServer server(8443, router, "cert.pem", "key.pem");
    server.run();

    return 0;
}
```

The certificate and key paths are resolved relative to the working directory, so run the binary
from the directory containing `cert.pem` and `key.pem`.

### 3. Test it

```bash
./build/secure_server        # run from the project root

curl -k https://localhost:8443/
# Hello over HTTPS!

curl -kv https://localhost:8443/
# ... "SSL connection using TLSv1.3" confirms the encrypted transport
```

The `-k` flag tells curl to accept the self-signed certificate. On startup, `HttpsServer`
validates that the private key matches the certificate and throws a descriptive error if not.

---

## Full API Reference

All types below are in the `dashapi` namespace and are available through
`#include <dashapi/dashapi.h>`.

### `class Router`

| Method | Signature | Description |
|--------|-----------|-------------|
| `get`          | `void get(const std::string& path, Handler)`          | Register a `GET` route |
| `post`         | `void post(const std::string& path, Handler)`         | Register a `POST` route |
| `put`          | `void put(const std::string& path, Handler)`          | Register a `PUT` route |
| `patch`        | `void patch(const std::string& path, Handler)`        | Register a `PATCH` route |
| `delete_route` | `void delete_route(const std::string& path, Handler)` | Register a `DELETE` route |
| `add_route`    | `void add_route(const std::string& method, const std::string& path, Handler)` | Register a route for any method |
| `use`          | `void use(Middleware)`                                | Register a middleware |
| `handle`       | `HttpResponse handle(const HttpRequest&)`             | Run the chain and produce a response |

Where `Handler` is `std::function<HttpResponse(const HttpRequest&)>` and `Middleware` is
`std::function<HttpResponse(const HttpRequest&, std::function<HttpResponse(const HttpRequest&)>)>`.

### `class HttpServer`

| Member | Signature | Description |
|--------|-----------|-------------|
| constructor | `HttpServer(int port, Router router)` | Bind to `port`, take ownership of the router |
| `run`       | `void run()`                          | Accept connections and serve forever |

### `class HttpsServer`

| Member | Signature | Description |
|--------|-----------|-------------|
| constructor | `HttpsServer(int port, Router router, const std::string& cert_path, const std::string& key_path)` | Bind to `port`, load TLS cert/key |
| `run`       | `void run()`                          | Accept TLS connections and serve forever |

### `class HttpRequest`

Fields: `method`, `path`, `version`, `headers`, `query_params`, `path_params`, `form_params`, `body`.

| Method | Signature | Description |
|--------|-----------|-------------|
| `json`            | `nlohmann::json json() const` | Parse `body` as JSON (throws on invalid) |
| `parse_form_body` | `void parse_form_body()`      | Populate `form_params` if the body is urlencoded |

### `class HttpResponse`

Fields: `status`, `headers`, `body`.

| Method | Signature | Description |
|--------|-----------|-------------|
| `set_json` | `void set_json(const nlohmann::json&)`                            | Set body + JSON Content-Type |
| `json`     | `static HttpResponse json(HttpStatus, const nlohmann::json&)`     | Build a full JSON response |
| `error`    | `static HttpResponse error(HttpStatus, const std::string&)`      | Build a `{"error": "..."}` response |

### `enum class HttpStatus`

`OK`, `Created`, `BadRequest`, `Unauthorized`, `Forbidden`, `NotFound`, `MethodNotAllowed`,
`InternalServerError`, `NotImplemented`.

`std::string status_text(HttpStatus)` returns the reason phrase.

---

## Testing

DashAPI is tested with [Catch2](https://github.com/catchorg/Catch2), run through CTest. Tests
cover the parser, routing, path/query/form parsing, middleware ordering, error handling, JSON
responses, and the keep-alive decision logic.

Build and run all tests:

```bash
cmake --build build
cd build
ctest --output-on-failure
```

Run the Catch2 binary directly for richer output and tag filtering:

```bash
./build/tests               # run everything
./build/tests "[params]"    # run only tests tagged [params]
./build/tests "[json]"      # run only JSON response tests
```

Available tags include `[parser]`, `[query]`, `[params]`, `[body]`, `[middleware]`,
`[errors]`, `[json]`, and `[keepalive]`.

### Writing a test

Tests are plain Catch2. For example:

```cpp
#include <catch2/catch_test_macros.hpp>
#include <dashapi/dashapi.h>

using namespace dashapi;

TEST_CASE("Path parameter is captured", "[params]") {
    Router router;

    router.get("/users/:id", [](const HttpRequest& request) {
        HttpResponse response;
        response.status = HttpStatus::OK;
        response.body = request.path_params.at("id");
        return response;
    });

    HttpRequest request;
    request.method = "GET";
    request.path = "/users/42";

    HttpResponse response = router.handle(request);

    REQUIRE(response.status == HttpStatus::OK);
    REQUIRE(response.body == "42");
}
```

Add new test files to the `add_executable(tests ...)` list in `CMakeLists.txt` and re-run CMake.

---

## Project Structure

```
DashAPI/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── dashapi-logo.svg
├── cert.pem                 # dev only, git-ignored
├── key.pem                  # dev only, git-ignored
├── include/
│   ├── dashapi/
│   │   └── dashapi.h        # umbrella header — the intended entry point
│   └── http/                # underlying public headers
│       ├── HttpRequest.h
│       ├── HttpResponse.h
│       ├── HttpParser.h
│       ├── HttpResponseBuilder.h
│       ├── HttpStatus.h
│       ├── Route.h
│       ├── Router.h
│       ├── HttpServer.h
│       └── HttpsServer.h
├── src/                     # implementation
│   ├── HttpParser.cpp
│   ├── HttpResponseBuilder.cpp
│   ├── HttpStatus.cpp
│   ├── HttpRequest.cpp
│   ├── Router.cpp
│   ├── HttpServer.cpp
│   └── HttpsServer.cpp
├── examples/                # runnable usage examples
│   ├── basic_server.cpp
│   └── secure_server.cpp
└── tests/                   # Catch2 tests
    ├── test_parser.cpp
    ├── test_middleware.cpp
    ├── test_handle_error.cpp
    ├── test_params.cpp
    ├── test_pathParam.cpp
    ├── test_body.cpp
    ├── test_json_response.cpp
    └── test_keepalive.cpp
```

Include the framework with `#include <dashapi/dashapi.h>` — the umbrella header re-exports the
underlying types (which live in the `http` namespace) into the `dashapi` namespace, so your code
uses `dashapi::Router`, `dashapi::HttpServer`, and so on.

---

## Roadmap

Planned and possible future work:

- **URL decoding** for query strings, path parameters, and form bodies (`%20`, `+`).
- **HTTP pipelining** — track bytes-consumed in the parser to split buffered requests cleanly.
- **Static file serving.**
- **Request/response size limits and timeouts** for hardening.
- **Structured, thread-safe logging** to replace raw `std::cout`.
- **CMake install target** so the library can be consumed via `find_package` / `FetchContent`.
- **Route groups / prefixes** (e.g. mounting a set of routes under `/api`).
- **An `App` facade** — an optional single object bundling the router and server
  (`App app; app.get(...); app.listen(8080);`) for an even more concise entry point.

---

## Contributing

Contributions are welcome. A good workflow:

1. Fork the repository and create a feature branch.
2. Add or update tests in `tests/` for your change.
3. Ensure `ctest --output-on-failure` passes.
4. Open a pull request describing the change and its motivation.

Please keep the codebase's style: clear, readable C++ with no unnecessary dependencies.

---

## License

DashAPI is released under the MIT License. See [LICENSE](LICENSE) for details.