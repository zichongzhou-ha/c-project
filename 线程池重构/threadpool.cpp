#include"threadpool.h"
#include<functional>
#include<thread>
#include<iostream>
const int TASK_MAX_THRESHHOLD = 4;

ThreadPool::ThreadPool()
	:initThreadSize_(0)//这里设为0:其实就是无所谓，在start函数里进行赋值
	,taskSize_(0)
	,taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD)
	,poolmode_(PoolMode::MODE_FIXED)
{

}
ThreadPool::~ThreadPool()
{

}
void ThreadPool::start(int initThreadSize)
{
	initThreadSize_ = initThreadSize;

	for (int i = 0; i < initThreadSize_; i++)
	{
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
		threads_.emplace_back(std::move(ptr));//这里不写move就算拷贝构造了--》那就不行了
	}

	for (int i = 0; i < initThreadSize_; i++)
	{
		threads_[i]->start();
	}

}
//void ThreadPool::setInitThreadSize(int size)
//{
//}
void ThreadPool::setMode(PoolMode mode)
{
	poolmode_ = mode;
}
void ThreadPool::setTaskQueMaxThreshHold(int threshhold)
{
	taskQueMaxThreshHold_ = threshhold;
}
Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {

	std::unique_lock<std::mutex>lock(taskQueMtx_);

	/*while (taskQue_.size() == taskQueMaxThreshHold_)
	{
		notFull_.wait(lock);
	}*/

	if (!notFull_.wait_for(lock, std::chrono::seconds(1), \
		[&]()->bool {return taskQue_.size() < taskQueMaxThreshHold_; }))
	{
		std::cerr << "task queue is full,submit task fail." << std::endl;
		return Result(sp, false);
	}

	taskQue_.emplace(sp);
	taskSize_++;

	notEmpty_.notify_all();
	return Result(sp, true);
}
void ThreadPool::threadFunc()
{
	for (;;)
	{
		std::shared_ptr<Task>sp;
		{
			std::unique_lock<std::mutex>lock(taskQueMtx_);

			std::cout << "tid: " << std::this_thread::get_id() << "尝试获取任务。。。" << std::endl;
			notEmpty_.wait(lock, [&]()->bool {return taskQue_.size() > 0; });
			std::cout << "tid: " << std::this_thread::get_id() << "获取任务成功。。。" << std::endl;

			sp = taskQue_.front();
			taskQue_.pop();
			taskSize_--;
			if (taskQue_.size() > 0)notEmpty_.notify_all();
			notFull_.notify_all();
		}
		if (sp != nullptr)
		{
			sp->exec();
		}
	}
}



////////////////////////////////////////////////////////////////////////////////


void Thread::start()
{

}

Thread::Thread(ThreadFunc func)
	:func_(func)
{
	std::thread t(func_);
	t.detach();//
}
Thread::~Thread(){}



Result::Result(std::shared_ptr<Task>task, bool isValid)
	:isValid_(isValid)
	,task_(task)
{
	task_->setResult(this);
}
Any Result::get()
{
	if (!isValid_)
	{
		return "";
	}
	sem_.wait();
	return std::move(any_);
}

void Result::setVal(Any any_)
{
	this->any_ = std::move(any_);
	sem_.post();
}

void Task::exec()
{
	if(result_!=nullptr)
	result_->setVal(run());
}
void Task::setResult(Result* res)
{
	result_ = res;
}


Task::Task()
	:result_(nullptr)
{

}