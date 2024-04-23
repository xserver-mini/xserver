# xserver

Is it difficult to develop C++ high-concurrency server applications? Come and use XServer, it allows you to develop C++ high-concurrency server programs in the simplest way.

XServer is a super simple server framework, developed by the developer over 8 years, to develop C++ server programs in the simplest way. Built with CMake, cross-platform design, it can run on Windows, Linux, Android and other platforms.


```
mkdir build
cd build
cmake ..
cmake --build .
./XServer
```

开发C++高并发服务器应用很难？快来使用XServer吧，它可以用最简单的方式开发C++高并发服务器程序。

XServer是一款超级简单的服务器框架，开发者历时8年打造，以最简单的方式开发C++服务器程序。CMake构建，跨平台设计，可以运行在Windows、Linux、Android等平台。


Without using coroutines, special methods can be used to achieve synchronous execution.
(Using loop to implement coroutine)
```C++
	auto request = createHttp();
	request->setUrl("http://127.0.0.1:8080/index.html");
	XHttpResponse* response = httpGet(request);
	if (!response)
	{
		XINFO("httpGet url failed. %s", request->url_.data());
		return;
	}
	XDEBUG("==>> code:%d", response->code_);
	XDEBUG("==>> head[%d]:%s", response->head_.size(), response->head_.data());
	XDEBUG("==>> body[%d]:%s", response->body_.size(), response->body_.data());
	XINFO("");
```
不使用协程，特殊方式实现同步执行