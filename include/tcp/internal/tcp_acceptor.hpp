#pragma once

#include <memory>
#include <functional>

#include <asio/basic_socket_acceptor.hpp>

#include <tcp/internal/tcp_functor.hpp>
#include <tcp/internal/tcp_shared_socket.hpp>
#include <common/io_context_thread_pool.hpp>

namespace gsio {
	namespace tcp {
		namespace internal {

			class TcpAcceptor : public asio::noncopyable,
				public std::enable_shared_from_this<TcpAcceptor>
			{
			private:
				asio::ip::tcp::acceptor mAcceptor;
				gsio::common::IoContextThreadPool::Ptr mIoContextThreadPool;

			public:
				using Ptr = std::shared_ptr<TcpAcceptor>;

				// listenContext 的 io_context 只给 tcp::acceptor 使用
				// ioContextThreadPool 中的 io_context 给新建立的连接使用
				static Ptr Make(asio::io_context& listenContext,
					const gsio::common::IoContextThreadPool::Ptr& ioContextThreadPool,
					const asio::ip::tcp::endpoint& endpoint)
				{
					class make_shared_enabler : public TcpAcceptor
					{
					public:
						make_shared_enabler(asio::io_context& listenContext,
							const gsio::common::IoContextThreadPool::Ptr& ioContextThreadPool,
							const asio::ip::tcp::endpoint& endpoint)
							:
							TcpAcceptor(listenContext, ioContextThreadPool, endpoint)
						{}
					};

					auto acceptor = std::make_shared<make_shared_enabler>(listenContext, ioContextThreadPool, endpoint);
					return std::static_pointer_cast<TcpAcceptor>(acceptor);
				}

				virtual ~TcpAcceptor()
				{
					close();
				}

				void startAccept(const SocketEstablishHandler& callback)
				{
					doAccept(callback);
				}

				void close()
				{
					mAcceptor.close();
				}

			private:
				TcpAcceptor(asio::io_context& listenContext,
					const gsio::common::IoContextThreadPool::Ptr& ioContextThreadPool,
					const asio::ip::tcp::endpoint& endpoint)
					: mIoContextThreadPool(ioContextThreadPool),
					mAcceptor(listenContext, endpoint)
				{}

				void doAccept(const SocketEstablishHandler& callback)
				{
					std::cout << "doAccept..." << std::endl;
					if (!mAcceptor.is_open())
					{
						std::cout << "acceptor is not open!" << std::endl;
						return;
					}

					auto& ioContext = mIoContextThreadPool->pickIoContext();
					auto sharedSocket = TcpSharedSocket::Make(asio::ip::tcp::socket(ioContext), ioContext);

					const auto self = shared_from_this();
					mAcceptor.async_accept(sharedSocket->socket(),
						[self, callback, sharedSocket, this](std::error_code ec) mutable
						{
							if (!ec)
							{
								// 了解 io_context 的 post，defer，dispatch 方法的区别
								sharedSocket->context().post([=]()
									{
										callback(std::move(sharedSocket->socket()));
									});
							}
							doAccept(callback);
						});
				}
			};

		}
	}
}