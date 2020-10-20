#pragma once

#include <vector>

#include <common/io_context_thread.hpp>

namespace gsio { namespace common {

	class IoContextThreadPool : public asio::noncopyable
	{
	private:
		std::mutex mPoolGuard;
		std::atomic_long mPickIoContextIndex;
		std::vector<std::shared_ptr<IoContextThread>> mIoContextThreadList;
		
	public:
		using Ptr = std::shared_ptr<IoContextThreadPool>;

		// poolSize defines the number of IoThread 
		// concurrencyHint defines the number of thread num to run io_context.run() which used to exec callback
		static Ptr Make(size_t poolSize, int concurrencyHint)
		{
			return std::make_shared<IoContextThreadPool>(poolSize, concurrencyHint);
		}

		IoContextThreadPool(size_t poolSize, int concurrencyHint)
			: mPickIoContextIndex(0)
		{
			if (poolSize == 0)
			{
				throw std::runtime_error("pool size is zero");
			}

			for (size_t i = 0; i < poolSize; i++)
			{
				mIoContextThreadList.emplace_back(std::make_shared<IoContextThread>(concurrencyHint));
			}
		}

		virtual ~IoContextThreadPool() noexcept
		{
			stop();
		}

		void  start(size_t threadNumEveryContext)
		{
			std::lock_guard<std::mutex> lck(mPoolGuard);

			for (const auto& ioContextThread : mIoContextThreadList)
			{
				ioContextThread->start(threadNumEveryContext);
			}
		}

		void  stop()
		{
			std::lock_guard<std::mutex> lck(mPoolGuard);

			for (const auto& ioContextThread : mIoContextThreadList)
			{
				ioContextThread->stop();
			}
		}

		asio::io_context& pickIoContext()
		{
			const auto index = mPickIoContextIndex.fetch_add(1, std::memory_order::memory_order_relaxed);
			return mIoContextThreadList[index % mIoContextThreadList.size()]->context();
		}

		std::shared_ptr<IoContextThread> pickIoContextThread()
		{
			const auto index = mPickIoContextIndex.fetch_add(1, std::memory_order::memory_order_relaxed);
			return mIoContextThreadList[index % mIoContextThreadList.size()];
		}
	};

} }