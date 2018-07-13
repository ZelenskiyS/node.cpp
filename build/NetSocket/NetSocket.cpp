// NetSocket.cpp : Defines the entry point for the console application.
//


#include "../../3rdparty/fmt/include/fmt/format.h"
#include "../../include/nodecpp/net.h"
#include "../../include/nodecpp/loop.h"

#include <functional>


using namespace std;
using namespace nodecpp;
using namespace fmt;

class MyServer;

class MySocket :public net::Socket {
	size_t recvSize = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;

public:
	MySocket() {}

	void onClose(bool hadError) override {
		print("onClose!\n");
	}

	void onConnect() override {
		print("onConnect!\n");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
			ok = write(ptr.get(), size);
			sentSize += size;
		}
	}

	void onData(Buffer& buffer) override {
		print("onData!\n");
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			end();
	}

	void onDrain() override {
		print("onDrain!\n");
	}

	void onEnd() override {
		print("onEnd!\n");
	}

	void onError() override {
		print("onError!\n");
	}
};


class MySocketLambda :public net::Socket {
	size_t recvSize = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;

public:
	MySocketLambda() {
		//preset handlers
		on<event::Close>(std::bind(&MySocketLambda::didClose, this, std::placeholders::_1));
//		on<event::Connect>(std::bind(&MySocketLambda::didConnect, this));
//		on<event::Data>(std::bind(&MySocket2::didData, this, std::placeholders::_1));
//		on<event::Drain>(std::bind(&MySocketLambda::didDrain, this));
		on<event::End>(std::bind(&MySocketLambda::didEnd, this));
		on<event::Error>(std::bind(&MySocketLambda::didError, this));
	}

	void didClose(bool hadError) {
		print("onClose!\n");
	}

	void didConnect() {
		print("onConnect!\n");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
			ok = write(ptr.get(), size, [] { print("onDrain!\n"); });
			sentSize += size;
		}
	}

	void didData(Buffer& buffer) {
		print("onData!\n");
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			end();
	}

	//void didDrain() {
	//	print("onDrain!\n");
	//}

	void didEnd() {
		print("onEnd!\n");
	}

	void didError() {
		print("onError!\n");
	}
};

class MyServerSocket :public net::Socket {
	size_t count = 0;
	MyServer* server = nullptr;

public:
	MyServerSocket(MyServer* server) :server(server) {}

	void onClose(bool hadError) override;

	void onConnect() override {
		print("onConnect!\n");
	}

	void onData(Buffer& buffer) override {
		print("onData!\n");
		++count;
		write(buffer.begin(), buffer.size());
	}

	void onDrain() override {
		print("onDrain!\n");
	}

	void onEnd() override {
		print("onEnd!\n");
		const char buff[] = "goodbye!";
		write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		end();
	}

	void onError() override {
		print("onError!\n");
	}
};



class MyServer :public net::Server {
	list<unique_ptr<net::Socket>> socks;
public:
	void onClose(bool hadError) override {
		print("onClose!\n");
	}
	void onConnection(unique_ptr<net::Socket> socket) override {
		print("onConnection!\n");
		socks.push_back(std::move(socket));
	}
	void onListening() override {
		print("onListening!\n");
	}

	void onError() override {
		print("onError!\n");
	}
	std::unique_ptr<net::Socket> makeSocket() override {
		return std::unique_ptr<net::Socket>(new MyServerSocket(this));
	}

	void closeMe(MyServerSocket* ptr) {
		for (auto it = socks.begin(); it != socks.end(); ++it) {
			if (it->get() == ptr) {
				socks.erase(it);
				return;
			}
		}
	}
};

inline
void MyServerSocket::onClose(bool hadError) {
	print("onClose!\n");
	if (server)
		server->closeMe(this);
}


int main()
{
	auto srv = net::createServer<MyServer>();
	srv->listen(2000, "127.0.0.1", 5);

//	auto cli2 = net::createConnection<MySocket>(2000, "127.0.0.1");
	MySocketLambda* cli = net::createConnection<MySocketLambda>(2000, "127.0.0.1", [&cli] {cli->didConnect(); });
	cli->on<event::Data>([&cli](Buffer& buffer) { cli->didData(buffer); });


	runLoop();

	return 0;
}


