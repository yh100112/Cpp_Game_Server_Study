#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

// stl은 기본적으로 멀티스레드 환경에서 동작하지 않는다.
vector<int32> v;

// mutual exclusive (상호배타적 - 내가 먼저 락 획득하면 내가 풀기 전까지 남이 사용할 수 없다)
mutex m;

// RAII (Resource Acquisition Is Initialization) - mutex를 자동으로 잠가주고 자동으로 해제해주는 래퍼 클래스
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
		//m.lock(); // 재귀적으로 락을 걸면 에러 발생 ->  recursive_lock을 사용하면 가능. 컨텐츠 개발할 때 재귀적 락이 개발에 용이한 경우가 많긴 하다. ( 복잡한 구조에서는 함수를 타고타고 그 안에서 락을 또하는경우가 많기 때문

		//LockGuard<std::mutex> lockGuard(m); // 안전하게 뮤텍스를 래핑해서 사용
		//std::lock_guard<std::mutex> lockGuard(m);
		std::unique_lock<std::mutex> uniqueLock(m, std::defer_lock); // lock_guard와 다른 점 : lock_guard처럼 만들자마자 잠기는 게 아니라 잠금 시점을 뒤로 미룰 수 있다. 나머지는 동일
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