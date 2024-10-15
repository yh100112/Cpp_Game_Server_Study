#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <future> // event, condition_variable보다는 사용성 떨어짐


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
	// 동기(synchronous) 실행
	//int64 sum = Calculate();
	//cout << sum << endl;

	// std::future
	{
		// 1) deferred -> lazy evaluation 지연해서 실행하세요 ( 이건 멀티스레드로 도는 거 아님! )
		// 2) async -> 별도의 스레드를 만들어서 실행하세요
		// 3) deffered || async -> 둘 중 알아서 골라주세요
		
		// 언젠가 미래에 결과물을 뱉어줄거야! (get)
		std::future<int64> future = std::async(std::launch::async, Calculate); // async : 멀티스레드 환경으로 되는 것 ( 스레드를 귀찮게 만들 필요 없이 내부적으로 그 구조를 사용해줌 )

		// TODO

		/*std::future_status status = future.wait_for(1ms);
		if (status == future_status::ready)*/

		int64 sum = future.get(); // 결과물이 이제서야 필요하다!
	}

	// std::promise
	{
		// 미래(std::future)에 결과물을 반환해줄꺼라 약속(std::promise)해줘~ (계약서?)
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

	// 결론)
	/*
	mutex, condition_variable까지 가지 않고 단순하게 단기 알바처럼 처리할 수 있는 애들을 구현하는 게 목표
	특히나, 한 번 발생하는 단발성 작업에 유용하다!
	-> 닭잡는데 소잡는 칼을 쓸 필요없다 ( 굳이 복잡한 구현을 쓸 필요 없다. )

	1) asnyc : 원하는 함수를 비동기적으로 실행
	2) promise : 결과물을 promise를 통해 future로 받아줌
	3) packaged task : 원하는 함수의 실행 결과를 packaged_task를 통해 future로 받아줌
	*/
}