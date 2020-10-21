#include <iostream>
#include <memory>

#include <tcp/tcp.hpp>

size_t packetSize = 10;

class EchoService : public gsio::tcp::TcpServerService
{
	void onRawSocket(asio::ip::tcp::socket&) override
	{
		std::cout << "onRawSocket" << std::endl;
	}

	void onConnected(gsio::tcp::SessionPtr session) override
	{
		std::cout << "onConnected" << std::endl;
	}

	size_t dataHandler(gsio::tcp::SessionPtr session, const char* buffer, size_t size) override
	{
		std::cout << buffer << std::endl;
		session->send(std::make_shared<std::string>(buffer, packetSize));
		return size;
	}

	void onClosed(gsio::tcp::SessionPtr session) override
	{
		std::cout << "onClosed" << std::endl;
	}
};

int main()
{
	auto service = std::make_shared<EchoService>();
	gsio::tcp::TcpServer svr(9999, size_t(1), 1);
	svr.WithService(service).Start(1);
	std::cin.get();
	svr.Stop();
	return 0;
}