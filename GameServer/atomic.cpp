#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>

// atomic atom(원자) - all or nothing
// 공유 데이터를 다룰 때 동시에 접근하지 않도록 해줌
atomic<int32> sum = 0;

void Add()
{
	for (int32 i = 0; i < 100'0000; i++)
	{
		//sum++;
		sum.fetch_add(1); // atomic일때는 명시적으로 이렇게 해주는게 좋음 
	}
}

void Sub()
{
	for (int32 i = 0; i < 100'0000; i++)
	{
		//sum--;
		sum.fetch_add(-1);
	}
}

int main()
{
	std::thread t1(Add);
	std::thread t2(Sub);
	t1.join();
	t2.join();

	cout << sum << endl;
}