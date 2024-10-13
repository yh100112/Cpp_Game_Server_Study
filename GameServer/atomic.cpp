#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>

// atomic atom(����) - all or nothing
// ���� �����͸� �ٷ� �� ���ÿ� �������� �ʵ��� ����
atomic<int32> sum = 0;

void Add()
{
	for (int32 i = 0; i < 100'0000; i++)
	{
		//sum++;
		sum.fetch_add(1); // atomic�϶��� ��������� �̷��� ���ִ°� ���� 
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