

#ifndef THREAD_POOL_TASK_QUEUE_H
#define THREAD_POOL_TASK_QUEUE_H

#include<queue>
#include<mutex>
#include<condition_variable>
#include<functional>
class task_queue
{
public:
	using task_type = std::function<void()>;

	task_queue() = default;
	task_queue(const task_queue&) = delete;
	task_queue(task_queue&&) = delete;
	task_queue& operator=(const task_queue&) = delete;
	task_queue& operator=(task_queue&&) = delete;  
public:
	void push(task_type&& task)
	{
		{
			//std::unique_lock<std::mutex>lock(_mtx);
			std::lock_guard<std::mutex>lock(_mtx);
			_qu.push(std::move(task));
		}
		_con.notify_one();
	}
	task_type pop()
	{
		std::unique_lock<std::mutex>lock(_mtx);
		_con.wait(lock, [this] {return  !_qu.empty() || _stop; });
		if (_stop)
		{
			return task_type();
		}
		task_type task = std::move(_qu.front());
		_qu.pop();
		return task;
	}
	bool empty()const
	{
		//std::unique_lock<std::mutex>lock(_mtx);
		std::lock_guard<std::mutex>lock(_mtx);
		return _qu.empty();
	}
	void stop()
	{
		{
			//std::unique_lock<std::mutex>lock(_mtx);
			std::lock_guard<std::mutex>lock(_mtx);

			_stop = true;
		}
		_con.notify_all();
	}
private:
	std::queue<task_type> _qu;
	mutable std::mutex _mtx;
	std::condition_variable _con;
	bool _stop = false;
};
#endif // !THREAD_POOL_TASK_QUEUE_H


//#ifndef THREAD_POOL_TASK_QUEUE_TEST_H
//#define THREAD_POOL_TASK_QUEUE_TEST_H
//
//#include<mutex>
//#include<condition_variable>
//#include<functional>
//#include<queue>
//class task_queue
//{
//	using task_type = std::function<void()>;
//public:
//	task_queue() = default;
//	task_queue(const task_queue&) = delete;
//	task_queue(task_queue&&) = delete;
//	task_queue& operator=(const task_queue&) = delete;
//	task_queue& operator=(task_queue&&) = delete;
//	void push(task_type&& task)
//	{
//		{
//			std::lock_guard<std::mutex>lock(_mtx);
//			_qu.push(std::move(task));
//		}
//		_con.notify_one();
//	}
//	task_type pop()
//	{
//		std::unique_lock<std::mutex>lock(_mtx);
//		_con.wait(lock, [this] {return !_qu.empty() || _stop; });
//		if (_stop)
//		{
//			return task_type();
//		}
//		task_type task = std::move(_qu.front());
//		_qu.pop();
//		return task;
//	}
//	bool empty()const
//	{
//		std::lock_guard<std::mutex>lock(_mtx);
//		return _qu.empty();
//	}
//	void stop()
//	{
//		{
//			std::lock_guard<std::mutex>lock(_mtx);
//			_stop = true;
//		}
//		_con.notify_all();
//	}
//private:
//	std::queue<task_type> _qu;
//	mutable std::mutex _mtx;
//	std::condition_variable _con;
//	bool _stop = false;
//};
//#endif