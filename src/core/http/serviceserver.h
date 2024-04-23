/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#pragma once

#include <vector>
#include <set>
#include <memory>
#include <unordered_map>
#include <string.h>
#include "openssl.h"
#include "httplib.h"
#include "../common.h"
#include "../socket/servicesocket.h"


////////////XServiceHttpServer//////////////////////
class XServiceHttpServer : public XServiceSocket
{
protected:
    class XClient : public XHttpClient
    {
    public:
        int fd_;
        int robotId_;
        bool isHttps_;
        XTlsContext* tls_;
        XSslCtx* sslCtx_;
        XTlsBuffer buffer_;
        XHttpRequest* request_;
        XServiceHttpServer* service_;

        bool isSendData_;
        XClient();
        ~XClient();
        bool start(XServiceHttpServer* service);
        void processing();
        void sendResponse();
        void sendRawBuffer();
        void sendBuffer();
        void open();
        void update(const char* data, size_t size);
        void onOpen();
        void onData(const char* data, size_t size);
        void close();
    };

    XSslCtx sslCtx_;
    bool isHttps_;
    std::unordered_map<int, XClient> mapClient_;

    virtual void onSocketAccept(int fd, int afd, const std::string& ip, int port);
    virtual void onSocketOpen(int fd);
    virtual void onSocketData(int fd, const char* data, size_t size);
    virtual void onSocketClose(int fd, const char* info);
    virtual void onSocketError(int fd, const char* info);
    virtual void onSocketWarning(int fd, const char* info);

public:
    XServiceHttpServer(XRobot* robot);
    virtual ~XServiceHttpServer();
    static 	XServiceHttpServer* New(XRobot* robot);

    void openSSL(const std::string& keyFile, const std::string& certFile);

    virtual bool onHttp(XHttpRequest& req, XHttpResponse& rep);

    friend struct XHttpResponse;
};

