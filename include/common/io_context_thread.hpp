#pragma once

#include <common/io_context_wrapper.hpp>

namespace gsio { namespace common {

	class IoContextThread : public asio::noncopyable
	{
	private:
		std::mutex mIoThreadGuard;
		IoContentWrapper mIoContentWrapper;
		std::vector<std::thread> mIoThreads;

	public:
		using Ptr = std::shared_ptr<IoContextThread>;

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