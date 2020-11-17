#pragma once

#include <common/io_context_wrapper.hpp>

namespace gsio { namespace common {

	// IoContextThread 包含n个网络worker线程
	// 每个线程都会执行io_context.run()
	class IoContextThread : public asio::noncopyable
	{
	private:
		std::mutex mIoThreadGuard;
		IoContentWrapper mIoContentWrapper;
		std::vector<std::thread> mIoThreads;

	public:
		using Ptr = std::shared_ptr<IoContextThread>;

		// concurrencyHint 是构造 io_context 时候使用
		// 1 代表多线程，非1代表多线程
		// https://blog.csdn.net/weixin_43827934/article/details/84590572
		explicit IoContextThread(int concurrencyHint)
			: mIoContentWrapper(concurrencyHint)
		{}

		explicit IoContextThread(asio::io_context& ioContext)
			: mIoContentWrapper(ioContext)
		{}

		virtual ~IoContextThread() noexcept
		{
			stop();
		}

		void start(size_t threadNum) noexcept
		{
			std::lock_guard<std::mutex> lck(mIoThreadGuard);
			if (threadNum == 0)
			{
				throw std::runtime_error("thread num is zero!");
			}

			if (!mIoThreads.empty())
			{
				return;
			}

			for (size_t i = 0; i < threadNum; ++i)
			{
				mIoThreads.emplace_back(std::thread([this]() {
					mIoContentWrapper.run();
					})
				);
			}
		}

		void stop() noexcept
		{
			std::lock_guard<std::mutex> lck(mIoThreadGuard);

			mIoContentWrapper.stop();
			for (auto& thread : mIoThreads)
			{
				try
				{
					thread.join();
				}
				catch (...)
				{
				}
			}

			mIoThreads.clear();
		}

		asio::io_context& context() const
		{
			return mIoContentWrapper.context();
		}

		IoContentWrapper& wrapperIoContext()
		{
			return mIoContentWrapper;
		}
	};

} }