
#include "serviceclient.h"


////////////XServiceHttpClient::XClient//////////////////////
XServiceHttpClient::XClient::XClient() 
    : fd_(-1), 
    robotId_(-1), 
    tls_(0), 
    sslCtx_(0), 
    service_(0)
{
    isClosed_ = false;
}

XServiceHttpClient::XClient::~XClient()
{ 
    if (tls_)
    {
        delete tls_;
        tls_ = 0;
    }
    if (request_)
    {
        request_->clear();
        //delete request_;
        //request_ = 0;
    }
}

bool XServiceHttpClient::XClient::start(XServiceHttpClient* service, XHttpRequest* request)
{
    XASSERT(service);
    XASSERT(request);
    XASSERT(sslCtx_);
    XASSERT(!service_);
    service_ = service;
    request_ = request;
    isClosed_ = false;

    request->resp()->clear();
#ifndef USE_OPEN_SSL
    assert(!request->isHttps_);
#endif
    fd_ = request->fd_;
    robotId_ = request->robotId_;
    request->resp()->fd_ = fd_;
    request->resp()->robotId_ = robotId_;
    //request->client_ = this;
    //request->resp()->client_ = this;

    if (!request->isHttps_)
        tls_ = 0;
    else
        tls_ = new XTlsContext(sslCtx_, false, request->host_.c_str());
    return true;
}

void XServiceHttpClient::XClient::sendRequest()
{
    auto& request = request_;
    request->encodeReqHeader();
    buffer_.clear();
   
    auto& head = request->head_;
    buffer_.push(head.data(), head.size());
    //
    auto& body = request->body_;
    if (body.size() > 0)
    {
        buffer_.push(body.data(), (int)body.size());
    }
    buffer_.push("\r\n", (int)strlen("\r\n"));
    if (buffer_.size() > 0)
    {
        sendBuffer();
    }
    else
    {
        assert(false);
    }
}

void XServiceHttpClient::XClient::open()
{
    //printf("[HTTPClient]open[%s:%d]\n", clientMsg_->request_->domain_.c_str(), clientMsg_->request_->port_);
    if (!tls_)
    {
        onOpen();
        return;
    }
    buffer_.clear();
    if (tls_->handshake(0, 0, &buffer_) > 0)
    {
        sendRawBuffer();
    }
    else
    {
        assert(buffer_.size() == 0);
    }
}

void XServiceHttpClient::XClient::update(const char* data, size_t size)
{
    if (isClosed_) return;
    if (!tls_)
    {
        onData(data, size);
        return;
    }
    if (!tls_->isFinished())
    {
        buffer_.clear();
        int ret = tls_->handshake(data, size, &buffer_);
        if (ret > 0)
        {
            if (buffer_.size() > 0)
            {
                sendRawBuffer();
            }
        }
        else if (ret == 0)
        {
            assert(buffer_.size() == 0);
            if (tls_->isFinished())
            {
                onOpen();

                buffer_.clear();
                tls_->read(0, 0, &buffer_);
                if (buffer_.size() > 0)
                {
                    onData((const char*)buffer_.data(), buffer_.size());
                }
            }
        }
        else
        {
            assert(buffer_.size() == 0);
        }
        return;
    }
    else
    {
        buffer_.clear();
        tls_->read(data, size, &buffer_);
        if (buffer_.size() > 0)
        {
            onData((const char*)buffer_.data(), buffer_.size());
        }
    }
    return;
}

void XServiceHttpClient::XClient::onOpen()
{
    sendRequest();
}

void XServiceHttpClient::XClient::onData(const char* data, size_t size)
{
    auto response = request_->resp();
    if (response->isFinish_)
    {
        return;
    }
    if (response->responseData(data, size))
    {
        response->isFinish_ = true;
        close();
    }
}

void XServiceHttpClient::XClient::sendBuffer()
{
    if (buffer_.size() == 0) return;
    if (!tls_)
    {
        service_->sendSocket(fd_, (const char*)buffer_.data(), (int)buffer_.size());
    }
    else
    {
        XTlsBuffer buffer;
        tls_->write((char*)buffer_.data(), buffer_.size(), &buffer);
        service_->sendSocket(fd_, (const char*)buffer.data(), (int)buffer.size());
    }
}

void XServiceHttpClient::XClient::sendRawBuffer()
{
    if (buffer_.size() == 0) return;

    service_->sendSocket(fd_, (const char*)buffer_.data(), buffer_.size());
}

void XServiceHttpClient::XClient::close()
{
    if (isClosed_) return;
    isClosed_ = true;

    if (callback_)
    {
        callback_(*request_, *request_->resp());
        return;
    }
    if (request_)
    {
        service_->onHttp(*request_, *request_->resp());
    }
}

////////////XServiceHttpClient//////////////////////
XServiceHttpClient::XServiceHttpClient(XRobot* robot)
    :XServiceSocket(robot),
    sslCtx_(false),
    curRequest_(0)
{
    assert(robot);
}

XServiceHttpClient::~XServiceHttpClient()
{
    auto iter = mapFdToClient_.begin();
    for (;iter != mapFdToClient_.end(); iter++)
        iter->second.close();

    mapFdToClient_.clear();
}

XServiceHttpClient* XServiceHttpClient::New(XRobot* robot)
{
    return new XServiceHttpClient(robot);
}

void XServiceHttpClient::onInit()
{
    //XDEBUG("===>>");
}

void XServiceHttpClient::onStart()
{
    //XDEBUG("===>>");
}

void XServiceHttpClient::onStop()
{
    //XDEBUG("===>>");
}

void XServiceHttpClient::onSocketData(int fd, const char* data, size_t size)
{
    auto iter = mapFdToClient_.find(fd);
    if (iter == mapFdToClient_.end())
    {
        closeSocket(fd);
        return;
    }
    auto& client = iter->second;
    assert(client.fd_ == fd);
    assert(client.robotId_ == robotId_);
    client.update(data, size);
}

void XServiceHttpClient::onSocketOpen(int fd)
{
    auto iter = mapFdToClient_.find(fd);
    if (iter == mapFdToClient_.end())
    {
        closeSocket(fd);
        return;
    }
    auto& client = iter->second;
    assert(client.fd_ == fd);
    assert(client.robotId_ == robotId_);
    client.open();
}

void XServiceHttpClient::onSocketClose(int fd, const char* info)
{
    auto iter = mapFdToClient_.find(fd);
    if (iter != mapFdToClient_.end())
    {
        auto& client = iter->second;
        assert(client.fd_ == fd);
        assert(client.robotId_ == robotId_);
        client.close();

        mapFdToClient_.erase(iter);
    }
    XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
}

void XServiceHttpClient::onSocketError(int fd, const char* info)
{
    auto iter = mapFdToClient_.find(fd);
    if (iter != mapFdToClient_.end())
    {
        auto& client = iter->second;
        assert(client.fd_ == fd);
        assert(client.robotId_ == robotId_);
        client.close();

        mapFdToClient_.erase(iter);
    }
    XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
}

void XServiceHttpClient::onSocketWarning(int fd, const char* info)
{
    XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
}

XHttpRequest* XServiceHttpClient::createHttp()
{
    if (!curRequest_)
    {
        curRequest_ = new XHttpRequest(true);
    }
    curRequest_->clear();
    return curRequest_;
}

bool XServiceHttpClient::sendHttp(XHttpRequest* request)
{
    return sendHttp(request, 0);
}

bool XServiceHttpClient::httpGet(XHttpRequest* request, const std::function<void(XHttpRequest&, XHttpResponse&)>& callback)
{
    request->method_ = "GET";
    return sendHttp(request, callback);
}

bool XServiceHttpClient::httpPost(XHttpRequest* request, const std::function<void(XHttpRequest&, XHttpResponse&)>& callback)
{
    request->method_ = "POST";
    return sendHttp(request, callback);
}

bool XServiceHttpClient::sendHttp(XHttpRequest* request, const std::function<void(XHttpRequest&, XHttpResponse&)>& callback)
{
    if (!request)
    {
        XASSERT(false);
        return false;
    }
    if (curRequest_ != request)
    {
        XASSERT(false);
        return false;
    }
    request->parseUrl();
    request->lookIp();
    if (request->ip().empty())
    {
        return false;
    }
#ifndef USE_OPEN_SSL
    if (request->isHttps_)
    {
        XERROR("Need open OpenSSL");
        return false;
    }
#endif
    int fd = connectSocket(request->ip(), request->port_);
    if (fd < 0)
    {
        return false;
    }
    auto& client = mapFdToClient_[fd];
    assert(client.fd_ == -1);
    request->fd_ = fd;
    request->robotId_ = robotId_;

    client.sslCtx_ = &sslCtx_;
    client.callback_ = callback;

    client.start(this, request);
    //curRequest_ = 0;
    return true;
}

void XServiceHttpClient::onHttp(XHttpRequest& req, XHttpResponse& rep)
{

}
