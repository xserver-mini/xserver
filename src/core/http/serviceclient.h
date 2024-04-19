/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#pragma once

#include <functional>
#include "openssl.h"
#include "httplib.h"
#include "../utils/common.h"
#include "../socket/servicesocket.h"


////////////XServiceHttpClient//////////////////////
class XServiceHttpClient : public XServiceSocket
{
protected:
    class XClient : public XHttpClient
    {
    public:
        int fd_;
        int robotId_;
        XServiceHttpClient* service_;
        std::function<void(XHttpRequest&, XHttpResponse&)> callback_;

        bool isClosed_;
        XTlsContext* tls_;
        XSslCtx* sslCtx_;
        XTlsBuffer buffer_;
        XHttpRequest* request_;

        XClient();
        ~XClient();
        bool start(XServiceHttpClient* service, XHttpRequest* request);
        void sendRequest();
        void sendRawBuffer();
        void sendBuffer();
        void open();
        void update(const char* data, size_t size);
        void onOpen();
        void onData(const char* data, size_t size);
        void close();
    };
    std::unordered_map<int, XClient> mapFdToClient_;

    XSslCtx sslCtx_;
public:
    XServiceHttpClient(XRobot* robot);
    virtual ~XServiceHttpClient();
    static XServiceHttpClient* New(XRobot* robot);

    virtual void onInit();
    virtual void onStart();
    virtual void onStop();

    virtual void onSocketOpen(int fd);
    virtual void onSocketData(int fd, const char* data, size_t size);
    virtual void onSocketClose(int fd, const char* info);
    virtual void onSocketError(int fd, const char* info);
    virtual void onSocketWarning(int fd, const char* info);

    XHttpRequest* createHttp();
    bool sendHttp(XHttpRequest* request);
    bool sendHttp(XHttpRequest* request, const std::function<void(XHttpRequest&, XHttpResponse&)>& callback);
    virtual void onHttp(XHttpRequest& req, XHttpResponse& rep);

protected:

    XHttpRequest* curRequest_;
};







