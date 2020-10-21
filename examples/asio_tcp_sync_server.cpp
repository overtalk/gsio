#include <iostream>
#include <asio.hpp>

int main()
{
	asio::io_service iosev;
	asio::ip::tcp::acceptor acceptor(iosev, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 9999));

	for (;;)
	{
		asio::ip::tcp::socket socket(iosev);

		std::cout << "wait for tcp connection .... " << std::endl;
		acceptor.accept(socket);

		auto remote_addr = socket.remote_endpoint().address();
		std::cout << "connected! address = " << remote_addr << std::endl;

		asio::error_code ec;
		socket.write_some(asio::buffer("hello world!\n"), ec);

		if (ec)
		{
			std::cout << "send error, msg = " << ec.message() << std::endl;
		}

		socket.close();
		std::cout << "close connection .... " << std::endl;
	}
}