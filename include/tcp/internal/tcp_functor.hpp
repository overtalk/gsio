#pragma once

#include <vector>
#include <iostream>
#include <functional>

#include <asio.hpp>

#include <tcp/internal/tcp_session.hpp>
#include <tcp/internal/tcp_acceptor.hpp>

namespace gsio { namespace tcp { namespace internal {

	using SocketEstablishHandler = std::function<void(asio::ip::tcp::socket)>;
	using SocketFailedConnectHandler = std::function<void()>;
	using SocketProcessingHandler = std::function<void(asio::ip::tcp::socket&)>;

} } }