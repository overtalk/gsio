#pragma once

#include <memory>

#include <asio.hpp>

#include <tcp/internal/tcp_session.hpp>
#include <tcp/internal/tcp_acceptor.hpp>
#include <common/io_context_thread_pool.hpp>

namespace gsio {
	namespace tcp {

		using SessionPtr = internal::TcpSession::Ptr;

		class TcpService
		{
		public:
			virtual void onRawSocket(asio::ip::tcp::socket&) = 0;
			virtual void onConnected(SessionPtr session) = 0;
			virtual size_t dataHandler(SessionPtr session, const char* data, size_t size) = 0;
			virtual void onClosed(SessionPtr session) = 0;
		};

		class TcpServer : public asio::noncopyable
		{
		private:
			internal::TcpAcceptor::Ptr mAcceptor;
			std::shared_ptr<TcpService> mTcpService;
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
				std::cout << "TcpServer init start" << std::endl;
				mAcceptor = internal::TcpAcceptor::Make(
					mAcceptorIoContext.context(),
					mSessionIoContextThreadPool,
					asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
				std::cout << "TcpServer init end" << std::endl;
			}

			TcpServer& WithRecvBufferSize(size_t size) noexcept
			{
				mRecvBufferSize = size;
				return *this;
			}

			TcpServer& WithService(std::shared_ptr<TcpService> service) noexcept
			{
				mTcpService = std::move(service);
				return *this;
			}

			void Start(size_t thread_num_per_context)
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

				mAcceptorIoContext.start(1);
				mSessionIoContextThreadPool->start(thread_num_per_context);

				mAcceptor->startAccept([=](asio::ip::tcp::socket socket)
					{
						mTcpService->onRawSocket(socket);
						mTcpService->onConnected(internal::TcpSession::Make(std::move(socket), mRecvBufferSize, dataHandler, closeHandler));
					});
			}

			void Stop()
			{
				mAcceptorIoContext.stop();
				mSessionIoContextThreadPool->stop();
			}
		};

	}
}