#pragma once

#include <memory>

#include <asio.hpp>

#include <tcp/internal/tcp_acceptor.hpp>

namespace gsio { namespace tcp {

	class TcpService	
	{
		virtual void onRawSocket(asio::ip::tcp::socket&) = 0;
		virtual void onConnected(TcpSession::Ptr session) = 0;
		virtual size_t dataHandler(TcpSession::Ptr session, const char* data, size_t size) = 0;
		virtual void onClosed(TcpSession::Ptr session) = 0;
	}

	class TcpServer : public asio::noncopyable
	{
	private:
		TcpAcceptor::Ptr mAcceptor;
		std::shared_ptr<TcpService> mTcpService;

	public:
		TcpServer& WithAcceptor(TcpAcceptor::Ptr acceptor) noexcept
		{
			mAcceptor = std::move(acceptor);
			return *this;
		}

		TcpServer& WithAcceptor(std::shared_ptr<TcpService> service) noexcept
		{
			mTcpService = std::move(service);
			return *this;
		}

		void Start()
		{
			if (mAcceptor == nullptr)
			{
				throw std::runtime_error("acceptor is nullptr");
			}
			if (mTcpService == nullptr)
			{
				throw std::runtime_error("service is nullptr");
			}

			internal::TcpSession::DataHandler dataHandler = std::bind(&TcpService::dataHandler, mTcpService, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			internal::TcpSession::ClosedHandler closeHandler = std::bind(&TcpService::onClosed, mTcpService, std::placeholders::_1);

			mAcceptor->startAccept([=](asio::ip::tcp::socket socket)
			{
				mTcpService->onRawSocket(socket);
				mTcpService->onConnected(TcpSession::Make(std::move(socket), 1024, dataHandler, closeHandler));
			});
		}
	};

} }