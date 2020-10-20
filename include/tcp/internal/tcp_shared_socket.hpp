#pragma once

#include <memory>

#include <asio.hpp>

namespace gsio { namespace tcp { namespace internal {

	class TcpAcceptor;
	class TcpConnector;

	class TcpSharedSocket : public asio::noncopyable
	{
	private:
		asio::ip::tcp::socket mSocket;
		asio::io_context& mIoContext;

	public:
		using Ptr = std::shared_ptr<TcpSharedSocket>;

		TcpSharedSocket(asio::ip::tcp::socket socket, asio::io_context& ioContext)
			: mSocket(std::move(socket)), mIoContext(ioContext)
		{}

		virtual ~TcpSharedSocket() = default;

		asio::ip::tcp::socket& socket()
		{
			return mSocket;
		}

		asio::io_context& context() const
		{
			return mIoContext;
		}

	private:
		static Ptr Make(asio::ip::tcp::socket socket, asio::io_context& ioContext)
		{
			return std::make_shared<TcpSharedSocket>(std::move(socket), ioContext);
		}

		friend TcpAcceptor;
		friend TcpConnector;
	};

} } }
