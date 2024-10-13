#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

// stl�� �⺻������ ��Ƽ������ ȯ�濡�� �������� �ʴ´�.
vector<int32> v;

// mutual exclusive (��ȣ��Ÿ�� - ���� ���� �� ȹ���ϸ� ���� Ǯ�� ������ ���� ����� �� ����)
mutex m;

// RAII (Resource Acquisition Is Initialization) - mutex�� �ڵ����� �ᰡ�ְ� �ڵ����� �������ִ� ���� Ŭ����
template<typename T>
class LockGuard
{
public:
	LockGuard(T& m)
	{
		_mutex = &m;
		_mutex->lock();
	}

	~LockGuard()
	{
		_mutex->unlock();
	}

private:
	T* _mutex;
};

void Push()
{
	for (int32 i = 0; i < 10000; i++)
	{
		//m.lock();
		//m.lock(); // ��������� ���� �ɸ� ���� �߻� ->  recursive_lock�� ����ϸ� ����. ������ ������ �� ����� ���� ���߿� ������ ��찡 ���� �ϴ�. ( ������ ���������� �Լ��� Ÿ��Ÿ�� �� �ȿ��� ���� ���ϴ°�찡 ���� ����

		//LockGuard<std::mutex> lockGuard(m); // �����ϰ� ���ؽ��� �����ؼ� ���
		//std::lock_guard<std::mutex> lockGuard(m);
		std::unique_lock<std::mutex> uniqueLock(m, std::defer_lock); // lock_guard�� �ٸ� �� : lock_guardó�� �����ڸ��� ���� �� �ƴ϶� ��� ������ �ڷ� �̷� �� �ִ�. �������� ����
		uniqueLock.lock();

		v.push_back(i);

		if (i == 5000)
		{
			//m.unlock();
			break;
		}
		
		//m.unlock();
		//m.unlock();
	}
}

int main()
{
	std::thread t1(Push);
	std::thread t2(Push);

	t1.join();
	t2.join();

	cout << v.size() << endl;
}