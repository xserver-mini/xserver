#pragma once

#include "service.h"
#include "utils/event.h"
#include "core/open/openjson.h"
#include "http_util.h"

namespace HttpAccept
{

class Handle
{
public:
    //  /index.html
    bool OnIndex(XHttpRequest& req, XHttpResponse& resp)
    {
        auto html = Dom::DomCreate();
        auto& body = html->child("body");
        auto& h1 = body.create("h1");
        h1 = "XServer is works, Thanks.";

        std::string buffer;
        html->echo(buffer);
        resp.response(200, ".html", buffer);
        resp.send();
        // return false, need rep->send()
        return false;
    }

    //  /api/stock
    bool OnApiStock(XHttpRequest& req, XHttpResponse& resp)
    {
        //{
        //    "code": "xxxxx",
        //    "data" : [
        //       {"time": "2023-07-18", "price" : 10}
        //       {"time": "2023-07-19", "price": 20}
        //    ]
        //}
        auto& code = req.params_["code"];

        open::OpenJson json;
        json["code"] = code;
        auto& nodeData = json["data"];

        auto& row0 = nodeData[0];
        row0["time"] = "2023-07-18";
        row0["price"] = 10;

        auto& row1 = nodeData[1];
        row1["time"] = "2023-07-19";
        row1["price"] = 20;

        auto& buffer = json.encode();
        resp.response(200, ".json", buffer);
        // return true, don't need rep->send();
        return true;
    }

    typedef bool (Handle::* HttpCall)(XHttpRequest&, XHttpResponse&);
    std::unordered_map<std::string, HttpCall> mapRouteHandles;
public:
    Handle()
    {
        mapRouteHandles["/"] = (HttpCall)&Handle::OnIndex;
        mapRouteHandles["/index.html"] = (HttpCall)&Handle::OnIndex;
        mapRouteHandles["/api/stock"] = (HttpCall)&Handle::OnApiStock;
    }

    ~Handle() { }
    bool onCallBack(XHttpRequest& req, XHttpResponse& resp)
    {
        XINFO("HTTP visit:%s:%d %s", req.ip().data(), req.port_, req.url_.data());
        if (req.url_ == "robots.txt")
        {
            resp.body_ = "User-agent: *\nDisallow: / \n";
            resp.code_ = 200;
            resp.ctype_ = "text/plain;charset=utf-8";
            return true;
        }
        HttpCall handle = 0;
        auto iter = mapRouteHandles.find(req.path_);
        if (mapRouteHandles.end() != iter)
        {
            handle = iter->second;
        }
        if (!handle)
        {
            handle = mapRouteHandles["/"];
        }
        return (this->*handle)(req, resp);
    }
};

};