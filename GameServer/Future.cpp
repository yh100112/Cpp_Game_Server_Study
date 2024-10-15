#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <future> // event, condition_variable���ٴ� ��뼺 ������


int64 Calculate()
{
	int64 sum = 0;

	for (int32 i = 0; i < 100'000; i++)
	{
		sum += i;
	}

	return sum;
}

void PromiseWorker(std::promise<string>&& promise)
{
	promise.set_value("Secret Message");
}

void TaskWorker(std::packaged_task<int64(void)>&& task)
{
	task();
}


int main()
{
	// ����(synchronous) ����
	//int64 sum = Calculate();
	//cout << sum << endl;

	// std::future
	{
		// 1) deferred -> lazy evaluation �����ؼ� �����ϼ��� ( �̰� ��Ƽ������� ���� �� �ƴ�! )
		// 2) async -> ������ �����带 ���� �����ϼ���
		// 3) deffered || async -> �� �� �˾Ƽ� ����ּ���
		
		// ������ �̷��� ������� ����ٰž�! (get)
		std::future<int64> future = std::async(std::launch::async, Calculate); // async : ��Ƽ������ ȯ������ �Ǵ� �� ( �����带 ������ ���� �ʿ� ���� ���������� �� ������ ������� )

		// TODO

		/*std::future_status status = future.wait_for(1ms);
		if (status == future_status::ready)*/

		int64 sum = future.get(); // ������� �������� �ʿ��ϴ�!
	}

	// std::promise
	{
		// �̷�(std::future)�� ������� ��ȯ���ٲ��� ���(std::promise)����~ (��༭?)
		std::promise<string> promise;
		std::future<string> future = promise.get_future();

		thread t(PromiseWorker, std::move(promise));

		string message = future.get();
		cout << message << endl;

		t.join();
	}

	// std::packaged_task
	{
		std::packaged_task<int64(void)> task(Calculate);
		std::future<int64> future = task.get_future();

		std::thread t(TaskWorker, std::move(task));

		int64 sum = future.get();
		cout << sum << endl;

		t.join();
	}

	// ���)
	/*
	mutex, condition_variable���� ���� �ʰ� �ܼ��ϰ� �ܱ� �˹�ó�� ó���� �� �ִ� �ֵ��� �����ϴ� �� ��ǥ
	Ư����, �� �� �߻��ϴ� �ܹ߼� �۾��� �����ϴ�!
	-> ����µ� ����� Į�� �� �ʿ���� ( ���� ������ ������ �� �ʿ� ����. )

	1) asnyc : ���ϴ� �Լ��� �񵿱������� ����
	2) promise : ������� promise�� ���� future�� �޾���
	3) packaged task : ���ϴ� �Լ��� ���� ����� packaged_task�� ���� future�� �޾���
	*/
}