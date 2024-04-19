
#include "serviceserver.h"

////////////XServiceHttpServer::XClient//////////////////////
XServiceHttpServer::XClient::XClient()
    : fd_(-1), 
    robotId_(-1),
    tls_(0), 
    sslCtx_(0),
    service_(0),
    isHttps_(false),
    request_(new XHttpRequest(false))
{
    isSendData_ = false;
}

XServiceHttpServer::XClient::~XClient()
{
    if (tls_)
    {
        delete tls_;
        tls_ = 0;
    }
    if (request_)
    {
        delete request_;
        request_ = 0;
    }
}

bool XServiceHttpServer::XClient::start(XServiceHttpServer* service)
{
    assert(sslCtx_);
    assert(service);
    service_ = service;

    request_->fd_ = fd_;
    request_->robotId_ = robotId_;
    request_->resp()->fd_ = fd_;
    request_->resp()->robotId_ = robotId_;
    request_->client_ = this;
    request_->resp()->client_ = this;

#ifdef USE_OPEN_SSL
#else
    assert(!request_->isHttps_);
#endif
    //XINFO("start[%s:%d]\n", request_->ip().c_str(), request_->port_);
    return true;
}

void XServiceHttpServer::XClient::processing()
{
    isSendData_ = false;
    request_->splitPaths();
    auto& response = *request_->resp();
    response.init();
    if (!service_->onHttp(*request_, response))
    {
        return;
    }
    sendResponse();
}

void XServiceHttpServer::XClient::sendResponse()
{
    if (isSendData_)
    {
        return;
    }
    isSendData_ = true;
    auto& response = *request_->resp();
    response.encodeRespHeader();
    buffer_.clear();
    buffer_.push(response.head_.data(), response.head_.size());
    buffer_.push(response.body_.data(), response.body_.size());
    buffer_.push("\r\n", strlen("\r\n"));
    if (buffer_.size() > 0)
    {
        sendBuffer();
    }
    else
    {
        assert(false);
    }
}

void XServiceHttpServer::XClient::open()
{
    //XINFO("open[%s:%d]\n", request_->ip().c_str(), request_->port_);
    if (!isHttps_)
    {
        tls_ = 0;
        onOpen();
    }
    else
        tls_ = new XTlsContext(sslCtx_, true);
}

void XServiceHttpServer::XClient::onOpen()
{
}

void XServiceHttpServer::XClient::onData(const char* data, size_t size)
{
    if (request_->isFinish_)
    {
        return;
    }
    if (request_->requestData(data, size))
    {
        request_->isFinish_ = true;
        processing();
    }
}

void XServiceHttpServer::XClient::update(const char* data, size_t size)
{
    //printf("\nOpenHttpAgent::Client::update[%s:%d] size = %lld\n", request_->ip().data(), request_->port_, size);
    if (!tls_)
    {
        onData(data, size);
        return;
    }
    if (!tls_->isFinished())
    {
        //printf("OpenHttpAgent::Client::update[%s:%d] handshake1, size:%lld\n", request_->ip().data(), request_->port_, size);
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
            if (buffer_.size() > 0)
            {
                sendRawBuffer();
            }
        }
        //printf("OpenHttpAgent::Client::update[%s:%d] handshake2, size:%lld\n", request_->ip().data(), request_->port_, size);
    }
    else
    {
        //printf("OpenHttpAgent::Client::update[%s:%d] read1, size:%lld\n", request_->ip().data(), request_->port_, size);
        buffer_.clear();
        tls_->read(data, size, &buffer_);
        //printf("OpenHttpAgent::Client::update[%d] read2, size:%lld\n", request_->port_, size);
        if (buffer_.size() > 0)
        {
            onData((const char*)buffer_.data(), buffer_.size());
        }
    }
}

void XServiceHttpServer::XClient::sendBuffer()
{
    if (buffer_.size() == 0) return;
    if (!tls_)
    {
        service_->sendSocket(fd_, (const char*)buffer_.data(), buffer_.size());
    }
    else
    {
        XTlsBuffer buffer;
        tls_->write((char*)buffer_.data(), buffer_.size(), &buffer);
        service_->sendSocket(fd_, (const char*)buffer.data(), buffer.size());
    }
}

void XServiceHttpServer::XClient::sendRawBuffer()
{
    if (buffer_.size() == 0) return;
    //printf("OpenHttpAgent::Client::[sendBuffer] size = %lld\n", buffer_.size());
    service_->sendSocket(fd_, (const char*)buffer_.data(), (int)buffer_.size());
}

void XServiceHttpServer::XClient::close()
{
    //printf("OpenHttpAgent::Client::close[%s:%d]\n", request_->ip().c_str(), request_->port_);
}

////////////XServiceHttpServer//////////////////////
XServiceHttpServer::XServiceHttpServer(XRobot* robot)
    :XServiceSocket(robot),
    sslCtx_(true),
    isHttps_(false)
{
    assert(robot);
}

XServiceHttpServer::~XServiceHttpServer()
{
}

XServiceHttpServer* XServiceHttpServer::New(XRobot* robot)
{
    return new XServiceHttpServer(robot);
}

void XServiceHttpServer::openSSL(const std::string& keyFile, const std::string& certFile)
{
#ifdef USE_OPEN_SSL
#else
    XASSERT(false);
#endif
    isHttps_ = true;
    sslCtx_.setCert(certFile.data(), keyFile.data());
}

void XServiceHttpServer::onSocketAccept(int fd, int afd, const std::string& ip, int port)
{
    XASSERT(fd == afd);
    //XDEBUG("%s[%d]fd=%d, afd=%d, %s:%d\n", serviceName_.data(), robotId_, fd, afd, ip.data(), port);
#ifdef USE_OPEN_SSL
#else
    XASSERT(!isHttps_);
#endif

    auto& client = mapClient_[fd];
    auto request = client.request_;
    request->clear();
    request->setIp(ip);
    request->port_ = port;
    request->isHttps_ = isHttps_;

    client.fd_ = afd;
    client.isHttps_ = isHttps_;
    client.robotId_ = robotId_;
    client.sslCtx_ = &sslCtx_;
    client.start(this);

    startSocket(fd);
}

void XServiceHttpServer::onSocketData(int fd, const char* data, size_t size)
{
    auto iter = mapClient_.find(fd);
    if (iter == mapClient_.end())
    {
        closeSocket(fd);
        return;
    }
    auto& client = iter->second;
    assert(client.fd_ == fd);
    assert(client.robotId_ == robotId_);
    client.update(data, size);
}

void XServiceHttpServer::onSocketClose(int fd, const char* info)
{
    auto iter = mapClient_.find(fd);
    if (iter != mapClient_.end())
    {
        auto& client = iter->second;
        assert(client.fd_ == fd);
        assert(client.robotId_ == robotId_);
        client.close();

        mapClient_.erase(iter);
    }
}

void XServiceHttpServer::onSocketOpen(int fd)
{
    auto iter = mapClient_.find(fd);
    if (iter == mapClient_.end())
    {
        closeSocket(fd);
        return;
    }
    auto& client = iter->second;
    assert(client.fd_ == fd);
    assert(client.robotId_ == robotId_);
    client.open();
}

void XServiceHttpServer::onSocketError(int fd, const char* info)
{
    XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
    auto iter = mapClient_.find(fd);
    if (iter != mapClient_.end())
    {
        auto& client = iter->second;
        assert(client.fd_ == fd);
        assert(client.robotId_ == robotId_);
        client.close();

        mapClient_.erase(iter);
    }
}

void XServiceHttpServer::onSocketWarning(int fd, const char* info)
{
    XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
}

bool XServiceHttpServer::onHttp(XHttpRequest& req, XHttpResponse& rep)
{
    return true;
}
