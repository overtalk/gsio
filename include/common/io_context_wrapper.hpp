#pragma once

#include <chrono>
#include <memory>
#include <functional>

#include <asio.hpp>

namespace gsio { namespace common {
	
	class IoContextThread;

	// wrapper of asio::io_context
	class IoContentWrapper : public asio::noncopyable {
	private:
		std::shared_ptr<asio::io_context> mTrickyIoContext;
		asio::io_context& mIoContext;

		friend IoContextThread;

		// concurrencyHint means the num of threads to do callback handlers
		explicit IoContentWrapper(int concurrencyHint)
			: mTrickyIoContext(std::make_shared<asio::io_context>(concurrencyHint))
			, mIoContext(*mTrickyIoContext)
		{}

		explicit IoContentWrapper(asio::io_context& ioContext)
			: mIoContext(ioContext)
		{}

	public:
		using Ptr = std::shared_ptr<IoContentWrapper>;

		virtual ~IoContentWrapper()
		{
			stop();
		}

		void run() const
		{
			// asio::io_service::work 可以防止 io_context 在没有io事件的情退出
			asio::io_service::work worker(mIoContext);
			while (!mIoContext.stopped())
			{
				// 调用这个方法的线程会阻塞
				mIoContext.run();
			}
		}

		void stop() const
		{
			mIoContext.stop();
		}

		asio::io_context& context() const
		{
			return mIoContext;
		}

		auto runAfter(std::chrono::nanoseconds timeout, std::function<void(void)> callback) const
		{
			auto timer = std::make_shared<asio::steady_timer>(mIoContext);
			timer->expires_from_now(timeout);
			timer->async_wait([callback = std::move(callback), timer](const asio::error_code& ec)
			{
				if (!ec)
				{
					callback();
				}
			});

			return timer;
		}
	};

} }