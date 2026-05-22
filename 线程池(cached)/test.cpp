#include<iostream>


#include"threadpool.h"

#include<chrono>

class Mytask :public Task
{
public:
	Mytask(int a,int b)
		:begin_(a)
		,end_(b)
	{}
	Any run()
	{
		std::cout << "tid: " << std::this_thread::get_id() << "begin " << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(2));
		long sum = 0;
		for (int i = begin_; i <= end_; i++)
		{
			sum += i;
		}
		std::cout << "tid: " << std::this_thread::get_id() << "end " << std::endl;
		return sum;
	}
private:
	int begin_;
	int end_;
};
int main()
{
	
	{
		ThreadPool pool;
		pool.setMode(PoolMode::MODE_CACHED);
		pool.start(4);
		Result res1 = pool.submitTask(std::make_shared<Mytask>(1, 100));
		int sum = res1.get().cast_<long>();
		std::cout << sum << std::endl;
	}

		//{
		////std::this_thread::sleep_for(std::chrono::seconds(5));
		//Result res2 = pool.submitTask(std::make_shared<Mytask>(1, 1000));
		//Result res3 = pool.submitTask(std::make_shared<Mytask>(1, 10000));
		//Result res4 = pool.submitTask(std::make_shared<Mytask>(1, 10000));

		//Result res5 = pool.submitTask(std::make_shared<Mytask>(1, 10000));
		//Result res6 = pool.submitTask(std::make_shared<Mytask>(1, 10000));
		//int sum = res1.get().cast_<long>();
		//std::cout << sum << std::endl;
		//sum = res2.get().cast_<long>();
		//std::cout << sum << std::endl;
		//sum = res3.get().cast_<long>();
		//std::cout << sum << std::endl;
		//sum = res4.get().cast_<long>();
		//std::cout << sum << std::endl;
	//}
	

	getchar();
	return 0;
}