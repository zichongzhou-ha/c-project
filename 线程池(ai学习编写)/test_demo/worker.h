
#ifndef THREAD_POOL_WORK_H
#define THREAD_POOL_WORK_H
#include<thread>
#include"task_queue.h"
class Worker_Thread
{
public:
	Worker_Thread(task_queue& tq)
		:tq_(tq)
		, thread_([this]() {
		this->run();
			})
	{

	}
	Worker_Thread(const Worker_Thread&) = delete;
	Worker_Thread& operator=(const Worker_Thread&) = delete;
	~Worker_Thread()
	{
		if (thread_.joinable())
		{
			thread_.join();
		}
	}
	void join()
	{
		if (thread_.joinable())
		{
			thread_.join();
		}
	}
	bool joinable() const
	{
		return thread_.joinable();
	}
	std::thread::id getid()const
	{
		return thread_.get_id();
	}
private:
	void run()
	{
		while (true)
		{
			auto task = tq_.pop();
			if (!task)
			{
				break;
			}
			task();
		}
	}
	task_queue& tq_;

	std::thread thread_;
};
#endif // !THREAD_POOL_WORK_H


//#ifndef THREAD_POOL_WORK_TEST_H
//#define THREAD_POOL_WORK_TEST_H
//#include<thread>
//#include"task_queue.h"
//class Work_Thread
//{
//public:
//	Work_Thread(task_queue& tq)
//		:tq_(tq)
//		, td_([this]() {this->run(); })
//	{
//
//	}
//	Work_Thread(const Work_Thread&) = delete;
//	Work_Thread& operator=(const Work_Thread&) = delete;
//	void join()
//	{
//		if (td_.joinable())
//		{
//			td_.join();
//		}
//	}
//	bool joinable()const
//	{
//		return td_.joinable();
//	}
//	~Work_Thread()
//	{
//		if (td_.joinable())
//		{
//			td_.join();
//		}
//	}
//private:
//	void run()
//	{
//		while (true)
//		{
//			auto task = tq_.pop();
//			if (!task)
//			{
//				break;
//			}
//			task();
//		}
//	}
//	task_queue& tq_;
//	std::thread td_;
//};
//#endif