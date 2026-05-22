
#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include<memory>
#include<vector>
#include<functional>
#include<future>
#include<type_traits>
#include"task_queue.h"
#include"worker.h"
class Thread_Pool
{
public:
	explicit Thread_Pool(std::size_t num)
		:task_queue_(std::make_unique<task_queue>())
	{
		workers_.reserve(num);
		for (int i = 0; i < num; i++)
		{
			workers_.push_back(std::make_unique<Worker_Thread>(*task_queue_));
		}
	}
	void shutdown()
	{
		if (!task_queue_)
		{
			return;
		}
		task_queue_->stop();
		for (auto& work : workers_)
		{
			if (work && work->joinable())
			{
				work->join();
			}
		}
		workers_.clear();
	}
	std::size_t getThreadCount()
	{
		return workers_.size();
	}
	~Thread_Pool()
	{
		shutdown();
	}
	template<typename F>
	std::future<typename std::invoke_result<F>::type> submit(F func)
	{
		using R = typename std::invoke_result<F>::type;

		auto promisePtr = std::make_shared<std::promise<R>>();
		std::future<R> future_ = promisePtr->get_future();

		task_queue::task_type taskwrapper = [promisePtr, funcwrapper = std::move(func)]() mutable {
			try
			{
				if constexpr (std::is_same_v<R, void>)
				{
					funcwrapper();
					promisePtr->set_value();
				}
				else
				{
					promisePtr->set_value(funcwrapper());
				}
			}
			catch (...)
			{
				promisePtr->set_exception(std::current_exception());
			}
		};
		task_queue_->push(std::move(taskwrapper));
		return future_;
	}

private:
	std::unique_ptr<task_queue>task_queue_;
	std::vector<std::unique_ptr<Worker_Thread>>workers_;
};
#endif // !THREAD_POOL_H
