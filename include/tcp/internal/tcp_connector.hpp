#pragma once

#include <memory>

#include <tcp/internal/tcp_functor.hpp>
#include <tcp/internal/tcp_shared_socket.hpp>
#include <common/io_context_thread_pool.hpp>

namespace gsio { namespace tcp { namespace internal {

	class TcpConnector
	{
	private:
		gsio::common::IoContextThreadPool::Ptr mIoContextThreadPool;

	public:
		using Ptr = std::shared_ptr<TcpConnector>;

		explicit TcpConnector(gsio::common::IoContextThreadPool::Ptr ioContextThreadPool)
			: mIoContextThreadPool(std::move(ioContextThreadPool))
		{}

		void asyncConnect(
			asio::ip::tcp::endpoint endpoint,
			std::chrono::nanoseconds timeout,
			const SocketEstablishHandler& successCallback,
			const SocketFailedConnectHandler& failedCallback,
			const std::vector<SocketProcessingHandler>& socketProcessingHandlerList)
		{
			wrapperAsyncConnect(mIoContextThreadPool->pickIoContextThread(),
				{ std::move(endpoint) },
				timeout,
				successCallback,
				failedCallback,
				socketProcessingHandlerList);
		}

		static void asyncConnect(
			const std::shared_ptr<gsio::common::IoContextThread>& ioContextThread,
			asio::ip::tcp::endpoint endpoint,
			std::chrono::nanoseconds timeout,
			const SocketEstablishHandler& successCallback,
			const SocketFailedConnectHandler& failedCallback,
			const std::vector<SocketProcessingHandler>& socketProcessingHandlerList)
		{
			wrapperAsyncConnect(ioContextThread,
				{ std::move(endpoint) },
				timeout,
				successCallback,
				failedCallback,
				socketProcessingHandlerList);
		}

	private:
		static void wrapperAsyncConnect(
			const gsio::common::IoContextThread::Ptr& ioContextThread,
			const std::vector<asio::ip::tcp::endpoint>& endpoints,
			std::chrono::nanoseconds timeout,
			const SocketEstablishHandler& successCallback,
			const SocketFailedConnectHandler& failedCallback,
			const std::vector<SocketProcessingHandler>& socketProcessingHandlerList)
		{
			auto sharedSocket = TcpSharedSocket::Make(
				asio::ip::tcp::socket(ioContextThread->context()),
				ioContextThread->context());

			auto timeoutTimer = ioContextThread->wrapperIoContext().runAfter(timeout, [=]()
				{
					failedCallback();
				});

			asio::async_connect(sharedSocket->socket(), endpoints,
				[=](std::error_code ec, const asio::ip::tcp::endpoint&)
				{
					timeoutTimer->cancel();
					if (ec)
					{
						if (failedCallback != nullptr)
						{
							failedCallback();
						}
						return;
					}

					for (const auto& handler : socketProcessingHandlerList)
					{
						handler(sharedSocket->socket());
					}

					if (successCallback != nullptr)
					{
						successCallback(std::move(sharedSocket->socket()));
					}
				});
		}
	};

} } }