#pragma once

#include <string>
#include <cstring>
#include <list>

#include <esp_http_server.h>
#include <espp/utils/macros.h>
#include <vector>

#include "buffer.h"
#include "wifi.h"

namespace espp {

class HttpResponse: public std::string{
public:
    explicit
    HttpResponse(httpd_req_t* req):
        _request(req)
    {
        reserve(1024);
    }

    HttpResponse() = delete;

    HttpResponse(const HttpResponse&) = delete;

    HttpResponse(HttpResponse&&) = delete;

    ~HttpResponse()
    {
        httpd_resp_send(_request, c_str(), size());
    }

    HttpResponse& operator<<(const Buffer& buffer)
    {
        append(buffer.charData(), buffer.length());
        return *this;
    }

    std::pair<std::string, bool> body(size_t max_length = 256)
    {
        std::vector<char> str;
        str.reserve(max_length);
        const auto length = httpd_req_recv(_request, str.data(), max_length);
        return {{str.data(), str.data() + length}, length < max_length};
    }

private:
    httpd_req_t* _request;
};

template<class Server>
class WebServerHandler: protected espp::TaskBase{
public:
    using View = void (Server::*)(HttpResponse&);
    struct ViewStorage{
        Server* server;
        View view;
    };

    struct Handler{
        std::string uri;
        http_method method;
        View view;
    };

    bool Start()
    {
        INFO << "Init web server";
        ESPP_CHECK(_server == nullptr);
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        DEBUG << "Start on port" << config.server_port;

        if(httpd_start(&_server, &config) != ESP_OK) {
            ERROR << "Can't start HTTP server";
            return false;
        }

        INFO << "Register handlers";
        auto* server = static_cast<Server*>(this);
        for(const Handler& handler: server->handlers) {
            Register(server, handler.view, handler.uri.c_str(), handler.method);
        }
        DEBUG << "Server started";
        return true;
    }

    void Register(Server* server, View view, const char* uri, http_method method = HTTP_GET)
    {
        DEBUG << "Register handler for" << uri;
        _views.push_back({server, view});
        httpd_uri_t uri_handler = {uri, method, WebServerHandler::ProcessResponse, &_views.back()};
        ESP_ERROR_CHECK(httpd_register_uri_handler(_server, &uri_handler));
        DEBUG << "Registration successful";
    }

protected:
    httpd_handle_t _server = nullptr;
    std::list<ViewStorage> _views;

    static
    esp_err_t ProcessResponse(httpd_req_t* request)
    {
        auto* handler = reinterpret_cast<ViewStorage*>(request->user_ctx);
        HttpResponse response(request);
        ((handler->server)->*(handler->view))(response);
        return ESP_OK;
    }
};

/**
 * Example
 *
 *      class WebServer: public WebServerHandler<WebServer>{
 *      public:
 *          std::vector<Handler> handlers = {
 *              {"/", HTTP_GET, &WebServer::Index},
 *              {"/", HTTP_POST, &WebServer::Update},
 *          };
 *      private:
 *          void Index(HttpResponse& response);
 *          void Update(HttpResponse& response);
 *      };
 */

}