/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#pragma once

#include <assert.h>
#include <memory>
#include <string.h>
#include <map>
#include <vector>
#include <string>

#include "core/robot/event.h"
#include "core/common.h"

class XHttpBuffer
{
public:
	size_t size_;
	size_t offset_;
	size_t cap_;
	size_t miniCap_;
	unsigned char* buffer_;
public:
	XHttpBuffer(size_t capacity = 256);
	~XHttpBuffer();

	void clear();
	inline size_t cap() { return cap_; }
	inline size_t size() { return size_; }
	unsigned char* data();
	unsigned char* clearResize(size_t size);
	inline void setCap(size_t capacity) { miniCap_ = capacity; }
	inline void toString(std::string& buffer)
	{
		buffer.clear();
		buffer.append((const char*)data(), size());
	}
	inline void operator=(std::string& data)
	{
		pushBack(data.data(), data.size());
	}

	inline void operator=(const std::string& data)
	{
		pushBack(data.data(), data.size());
	}

	int64_t pushBack(const void* data, size_t len);
	inline int64_t pushBack(const std::string& data)
	{
		return pushBack(data.data(), data.size());
	}
	int64_t popFront(void* data, size_t len);
	int64_t popBack(void* data, size_t len);
	inline int64_t popFront(std::string& data, size_t len)
	{
		data.resize(len);
		return popFront((void*)data.data(), data.size());
	};
};

////////////XHttp//////////////////////
class XHttpClient
{

};

struct XHttp
{
    int fd_;
    int uid_;
    int code_;
    int clen_;
    int offset_;
    bool isReq_;
    bool isHttps_;
    bool isChunked_;
    bool isClient_;
    bool isRemote_;
    bool isFinish_;
    int robotId_;
    unsigned int port_;
    std::string domain_;
    std::string method_;
    std::string path_;
    std::string host_;
    std::string url_;
    std::string ctype_;
    std::string body_;
    XHttpBuffer head_;
    XHttpBuffer buffer_;
    std::vector<std::string> paths_;
    std::map<std::string, std::string> params_;
    std::map<std::string, std::string> headers_;
    XHttpClient* client_;

    void splitPaths();
    const std::string& lookIp();
    inline const std::string& ip() { return ip_; }
    inline void setIp(const std::string& ip) { ip_ = ip; }
    inline void setUrl(const std::string& url) { url_ = url; }

    inline std::string& getBody() { return body_; }
    inline void getHead(std::string& head) { head.append((const char*)head_.data(), head_.size()); }
    inline std::string& setParam(const std::string& key) { return params_[key]; }
    std::string& header(const std::string& key);
    bool hasHeader(const std::string& key);
    void removeHeader(const std::string& key);
    inline std::string& operator[](const std::string& key) { return header(key); }

    XHttp();
    void parseUrl();
    void encodeReqHeader();
    void encodeRespHeader();
    virtual void decodeReqHeader();
    void decodeRespHeader();
    bool responseData(const char* data, size_t size);
    bool requestData(const char* data, size_t size);
    inline void operator=(const std::string& url) { url_ = url; }
    void setContentType(const std::string& ctype);

    void clear()
    {
        code_ = -1;
        clen_ = 0;
        port_ = 0;
        isHttps_ = false;
        isChunked_ = false;
        isRemote_ = false;
        isClient_ = false;
        isFinish_ = false;
        client_ = 0;
 
        domain_.clear();
        method_.clear();
        path_.clear();
        host_.clear();
        url_.clear();
        ctype_.clear();
        body_.clear();
        head_.clear();
        buffer_.clear();
        paths_.clear();
        params_.clear();
        headers_.clear();
    }

protected:
    static const std::string GetContentType(const char* fileExt);

private:
    std::string ip_;
    static const std::map<std::string, std::string> ContentTypes_;
};

////////////XHttpResponse//////////////////////
struct XHttpResponse : public XHttp
{
    XHttpResponse() : XHttp() { isReq_ = false; }
    virtual void decodeReqHeader();
    void init();
    void response(const char* ctype, const char* buffer, size_t len);
    void response(const char* ctype, const std::string& buffer);
    void response(int code, const std::string& ctype, const std::string& buffer);
    void response404Html();
    void send();
    
};

////////////XHttpRequest//////////////////////
class XHttpRequest : public XHttp
{
    XHttpResponse response_;
public:
    XHttpRequest(bool isClient = true) : XHttp(), listenPort_(0) 
    { 
        isReq_ = true; if (isClient) init(); 
    }

    XHttpResponse* resp() { return &response_; }

    void init()
    {
        port_ = 80;
        headers_["accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng";
        headers_["user-agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36(KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36";
        headers_["cache-control"] = "max-age=0";
        //headers_["accept-encoding"] = "gzip,deflate";
        headers_["accept-language"] = "en-us,zh;q=0.9";
        headers_["connection"] = "close";
    }
    int listenPort_;
};



