#pragma once

#include <chrono>
#include <memory>

#include <asio.hpp>

#include <tcp/internal/tcp_session.hpp>
#include <tcp/internal/tcp_acceptor.hpp>
#include <tcp/internal/tcp_connector.hpp>
#include <common/io_context_thread_pool.hpp>

namespace gsio {
	namespace tcp {

		using SessionPtr = internal::TcpSession::Ptr;

		class TcpServerService
		{
		public:
			virtual void onRawSocket(asio::ip::tcp::socket&) = 0;
			virtual void onConnected(SessionPtr session) = 0;
			virtual size_t dataHandler(SessionPtr session, const char* data, size_t size) = 0;
			virtual void onClosed(SessionPtr session) = 0;
		};

		class TcpClientService : public TcpServerService
		{
			// client only
			virtual void onConnectFail() = 0;
		};

		class TcpServer : public asio::noncopyable
		{
		private:
			internal::TcpAcceptor::Ptr mAcceptor;
			std::shared_ptr<TcpServerService> mService;

			// io_context
			common::IoContextThread mAcceptorIoContext;
			common::IoContextThreadPool::Ptr mSessionIoContextThreadPool;

			// session about 
			size_t mRecvBufferSize{ 1024 };


		public:
			TcpServer(int port, size_t poolSize, int concurrencyHint)
				: mAcceptorIoContext(1),
				mSessionIoContextThreadPool(common::IoContextThreadPool::Make(poolSize, concurrencyHint))
			{
				mAcceptor = internal::TcpAcceptor::Make(
					mAcceptorIoContext.context(),
					mSessionIoContextThreadPool,
					asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
			}

			TcpServer& WithRecvBufferSize(size_t size) noexcept
			{
				mRecvBufferSize = size;
				return *this;
			}

			TcpServer& WithService(std::shared_ptr<TcpServerService> service) noexcept
			{
				mService = std::move(service);
				return *this;
			}

			void Start(size_t thread_num_per_context)
			{
				if (mAcceptor == nullptr)
				{
					throw std::runtime_error("acceptor is nullptr");
				}
				if (mService == nullptr)
				{
					throw std::runtime_error("server service is nullptr");
				}

				auto dataHandler = std::bind(&TcpServerService::dataHandler, 
					mService, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
				auto closeHandler = std::bind(&TcpServerService::onClosed, mService, std::placeholders::_1);

				mAcceptorIoContext.start(1);
				mSessionIoContextThreadPool->start(thread_num_per_context);

				mAcceptor->startAccept([=](asio::ip::tcp::socket socket)
					{
						mService->onRawSocket(socket);
						mService->onConnected(internal::TcpSession::Make(std::move(socket), mRecvBufferSize, dataHandler, closeHandler));
					});
			}

			void Stop()
			{
				mAcceptorIoContext.stop();
				mSessionIoContextThreadPool->stop();
			}
		};

		class Tcpclient : public asio::noncopyable
		{
		private:
			internal::TcpConnector mConnector;
			std::shared_ptr<TcpClientService> mService;

			asio::ip::tcp::endpoint mEndpoint;
			std::chrono::nanoseconds mTimeout = std::chrono::seconds(10);
			
			// io_context 
			common::IoContextThreadPool::Ptr mSessionIoContextThreadPool;

		public:
			Tcpclient& WithTimeout(std::chrono::nanoseconds timeout) noexcept
			{
				mTimeout = timeout;
				return *this;
			}

			Tcpclient& WithEndpoint(asio::ip::tcp::endpoint endpoint) noexcept
			{
				mEndpoint = std::move(endpoint);
				return *this;
			}

			void asyncConnect()
			{
				if (!mConnector)
				{
					throw std::runtime_error("connector is empty");
				}
				if (mService == nullptr)
				{
					throw std::runtime_error("server service is nullptr");
				}

				//mSocketOption.establishHandler =
				//	[option = Option()](asio::ip::tcp::socket socket)
				//{
				//	const auto session = TcpSession::Make(
				//		std::move(socket),
				//		option.recvBufferSize,
				//		option.dataHandler,
				//		option.closedHandler);
				//	for (const auto& callback : option.establishHandlers)
				//	{
				//		callback(session);
				//	}
				//};

				mConnector->asyncConnect(
					mEndpoint,
					mTimeout,
					mSocketOption.establishHandler,
					mSocketOption.failedHandler,
					mSocketOption.socketProcessingHandlers);
			}
		};
	}
}