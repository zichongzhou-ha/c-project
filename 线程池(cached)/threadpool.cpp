#include"threadpool.h"
#include<functional>
#include<thread>
#include<iostream>
const int TASK_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_THRESHHOLD = 10;
const int THREAD_MAX_IDLE_TIME = 10;
ThreadPool::ThreadPool()
	:initThreadSize_(0)//这里设为0:其实就是无所谓，在start函数里进行赋值
	,taskSize_(0)
	,taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD)
	,poolmode_(PoolMode::MODE_FIXED)
	,isPoolRunning_(false)
	,idleThreadSize_(0)
	,threadSizeThreshHold_(THREAD_MAX_THRESHHOLD)
	,curThread_Size_(0)
{

}
void ThreadPool::setThreadSizeThreshHold(int threshhold)
{
	if (checkRunningState())return;
	if(poolmode_==PoolMode::MODE_CACHED)
	threadSizeThreshHold_ = threshhold;
}

ThreadPool::~ThreadPool()
{
	isPoolRunning_ = false;
	std::unique_lock<std::mutex>lock(taskQueMtx_);
	notEmpty_.notify_all();
	exitCond_.wait(lock, [&]()->bool {return threads_.size() == 0; });


}
void ThreadPool::start(int initThreadSize)
{
	isPoolRunning_ = true;
	initThreadSize_ = initThreadSize;
	curThread_Size_ = initThreadSize;
	for (int i = 0; i < initThreadSize_; i++)
	{
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this,std::placeholders::_1));
		int threadId = ptr->getId();
		threads_.emplace(threadId, std::move(ptr));
		//threads_.emplace_back(std::move(ptr));//这里不写move就算拷贝构造了--》那就不行了
	}

	for (int i = 0; i < initThreadSize_; i++)
	{
		threads_[i]->start();
		idleThreadSize_++;
	}

}
//void ThreadPool::setInitThreadSize(int size)
//{
//}
bool ThreadPool::checkRunningState()const
{
	return isPoolRunning_;
}

void ThreadPool::setMode(PoolMode mode)
{
	if (checkRunningState())return;
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

	if (poolmode_ == PoolMode::MODE_CACHED && taskSize_ > idleThreadSize_&&curThread_Size_<threadSizeThreshHold_)
	{
		std::cout << "create new thread:" << std::this_thread::get_id() << std::endl;

		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		int threadId = ptr->getId();
		threads_.emplace(threadId, std::move(ptr));
		curThread_Size_++;

		threads_[threadId]->start();  // 启动新创建的线程
		idleThreadSize_++;             // 增加空闲线程计数
	}
	return Result(sp, true);
}
void ThreadPool::threadFunc(int threadId)
{
	auto lastTime = std::chrono::high_resolution_clock().now();
	for(;;)
	{
		std::shared_ptr<Task>sp;
		{
			std::unique_lock<std::mutex>lock(taskQueMtx_);

			std::cout << "tid: " << std::this_thread::get_id() << "尝试获取任务。。。" << std::endl;

			while (taskQue_.size() == 0)
			{
				if (!isPoolRunning_)
				{
					threads_.erase(threadId);

					std::cout << "threadid: " << std::this_thread::get_id() << " exit" << std::endl;
					exitCond_.notify_all();
					return;
				}
				if (poolmode_ == PoolMode::MODE_CACHED)
				{
					if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1)))
					{
						auto now = std::chrono::high_resolution_clock().now();
						auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
						if (dur.count() >= THREAD_MAX_IDLE_TIME && curThread_Size_ > initThreadSize_)
						{
							threads_.erase(threadId);
							curThread_Size_--;
							idleThreadSize_--;
							std::cout << "threadid: " << std::this_thread::get_id() << " exit" << std::endl;
							return;
						}
					}
				}
				else
				{
					notEmpty_.wait(lock);

				}
				
			
			}
		
			
			idleThreadSize_--;
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
		idleThreadSize_++;
		lastTime = std::chrono::high_resolution_clock().now();

	}

	

}



////////////////////////////////////////////////////////////////////////////////

int Thread::generateId = 0;
void Thread::start()
{
	std::thread t(func_, threadId_);
	t.detach();
}

Thread::Thread(ThreadFunc func)
	: func_(func)
	,threadId_(generateId++)

{
	//std::thread t(func_);
	//t.detach();//
}
Thread::~Thread(){}
int  Thread::getId()const
{
	return threadId_;
}



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