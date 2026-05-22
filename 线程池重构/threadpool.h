#pragma once
#ifndef THREADPOOL_H
#define	THREADPOOL_h

#include<vector>
#include<queue>
#include<memory>
#include<atomic>
#include<mutex>
#include<condition_variable>
#include<functional>

//class Any
//{
//public:
//	Any() = default;
//	~Any() = default;
//	Any(const Any&) = delete;
//	Any& operator=(const Any&) = delete;
//	Any(Any&&) = default;
//	Any& operator=(Any&&) = default;
//	template<typename T>
//	Any(T data):base_(std::make_unique<Derive<T>>(data)){}
//
//	template<typename T>
//	T cast_()
//	{
//		Derive<T>* pd = dynamic_cast<Derive<T>>(base_.get());
//		if (pd == nullptr)
//		{
//			throw "type is unmatch!";
//		}
//		return pd->data_;
//	}
//private:
//	class Base
//	{
//	public:
//		virtual ~Base() = default;
//	};
//
//	template<class T>
//	class Derive:public Base
//	{
//	public:
//		Derive(T data) :data_(data)
//		{}
//
//		T data_;
//	};
//	std::unique_ptr<Base> base_;
//};
class Any
{
public:
	Any() = default;
	~Any() = default;
	Any(const Any&) = delete;
	Any& operator=(const Any&) = delete;
	Any(Any&&) = default;
	Any& operator=(Any&&) = default;

	template<typename T>
	Any(T data) : base_(std::make_unique<Derive<T>>(data)) {}

	template<typename T>
	T cast_()
	{
		// 用 typeid 比较，不依赖 dynamic_cast
		auto& base_ref = *base_;
		if (typeid(base_ref) != typeid(Derive<T>))
		{
			throw "type is unmatch!";
		}
		// 类型匹配后，用 static_cast 安全转换
		return static_cast<Derive<T>*>(base_.get())->data_;
	}

private:
	class Base
	{
	public:
		virtual ~Base() = default;
	};

	template<class T>
	class Derive : public Base
	{
	public:
		Derive(T data) : data_(data) {}
		T data_;
	};

	std::unique_ptr<Base> base_;
};
class Semaphore
{
public:
	Semaphore(int limit = 0)
		:resLimit_(limit)
	{

	}
	~Semaphore() = default;
	void wait()
	{
		std::unique_lock<std::mutex>lock(mtx_);
		cond_.wait(lock, [&]()->bool {return resLimit_ > 0; });
		resLimit_--;
	}
	void post()
	{
		std::unique_lock<std::mutex>lock(mtx_);
		resLimit_++;
		cond_.notify_all();

	}
private:
	int resLimit_;
	std::mutex mtx_;
	std::condition_variable cond_;
};
class Task;
class Result
{
public:
	Result(std::shared_ptr<Task>task, bool isValid = true);
	~Result() = default;
	Any get();
	void setVal(Any any_);
private:
	Any any_;
	Semaphore sem_;
	std::shared_ptr<Task>task_;
	std::atomic_bool isValid_;
};
class Thread
{
public:
	using ThreadFunc = std::function<void()>;
	void start();
	Thread(ThreadFunc func);
	~Thread();
private:
	ThreadFunc func_;
};
enum class PoolMode
{
	MODE_FIXED,
	MODE_CACHED,
};
class Task
{
public:
	void exec();
	void setResult(Result* res);
	Task();
	~Task() = default;
	virtual Any run() = 0;

public:
	Result* result_;
};
class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

	void start(int size=4);
	//void setInitThreadSize(int size);
	void setMode(PoolMode mode);
	void setTaskQueMaxThreshHold(int threshhold);
	Result submitTask(std::shared_ptr<Task> sp);

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:
	void threadFunc();
private:
	std::vector<std::unique_ptr<Thread>>threads_;
	int initThreadSize_;

	std::queue<std::shared_ptr<Task>>taskQue_;
	std::atomic_int taskSize_;
	int taskQueMaxThreshHold_;

	std::mutex taskQueMtx_;
	std::condition_variable notFull_;
	std::condition_variable notEmpty_;

	PoolMode poolmode_;
};

#endif
