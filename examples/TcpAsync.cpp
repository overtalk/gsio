#include <memory>
#include <string>
#include <thread>
#include <iostream>
#include <functional>

#include <asio.hpp>

int port = 9998;

struct DemoService
{
private:
	asio::io_service& mIoService;
	asio::ip::tcp::acceptor mAcceptor;

public:
	using socketPtr = std::shared_ptr<asio::ip::tcp::socket>;

	DemoService(asio::io_service& iosev)
		: mIoService(iosev),
		mAcceptor(iosev, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{}

	void start()
	{
		socketPtr pSocket(new asio::ip::tcp::socket(mIoService));
		mAcceptor.async_accept(*pSocket,
			std::move(std::bind(&DemoService::acceptHandler, this, pSocket, std::placeholders::_1))
		);
	}

	void acceptHandler(socketPtr pSocket, asio::error_code ec)
	{
		if (ec)
		{
			std::cout << "accept error, msg = " << ec.message() << std::endl;
			return;
		}

		// 继续accept其他的connection
		start();

		std::cout << "connected! remote addr = " << pSocket->remote_endpoint() << std::endl;

		std::shared_ptr<std::string> pStr(new std::string("hello async world!\n"));
		std::cout << "bytes to send : " << pStr->size() << std::endl;
		pSocket->async_write_some(asio::buffer(*pStr),
			std::move(std::bind(&DemoService::writeCallback, this, pStr, std::placeholders::_1, std::placeholders::_2))
		);
	}

	void writeCallback(std::shared_ptr<std::string> pStr, asio::error_code ec, size_t bytesTransferred)
	{
		if (ec)
		{
			std::cout << "send fail! errmsg = " << ec.message() << std::endl;
			return;
		}

		std::cout << "current thread id = " << std::this_thread::get_id() << std::endl;
		std::cout << "send success!, bytesTransferred = " << bytesTransferred << std::endl;

	}
};

int main()
{
	asio::io_service ioSev;
	DemoService sev(ioSev);

	std::cout << "start listen on port " << port << std::endl;
	sev.start();

	std::thread thrd([&ioSev]() {
		std::cout << "sub thread id = " << std::this_thread::get_id() << std::endl;
		ioSev.run();
		});
	thrd.detach();

	std::cout << "main thread id = " << std::this_thread::get_id() << std::endl;
	ioSev.run();
	return 0;
}