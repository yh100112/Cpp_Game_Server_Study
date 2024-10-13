#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

class SpinLock
{
public:
	void lock()
	{
		bool expected = false; // 초기값
		bool desired = true; // 초기값

		// CAS 의사코드
		//if (_locked == expected) // lock이 false면
		//{
		//	expected = _locked;
		//	_locked = desired; // lock을 true로 바꿔줌
		//	return true; // lock을 설정해주자마자 스핀락 대기 빠져나옴
		//}
		//else
		//{
		//	expected = _locked;
		//	return false;
		//}

		// CAS ( Compare-And-Swap ) : 스핀락 대기와 락 설정해주는걸 atomic하게 한번에 해주기 위해 필요 (옳은 스핀락)
		while (_locked.compare_exchange_strong(expected, desired) == false)
		{
			expected = false; // CAS 사용시 매번 expected에 locked가 들어가서 갱신되므로 초기값인 false로 다시 세팅해줘야 함

			// sleep 방식 ( 랜덤 메타 ) - 커널 모드로 가게 함  (system call - os에게 요청하는 부분)
			//1. this_thread::sleep_for(std::chrono::milliseconds(100));
			//2. this_thread::sleep_for(100ms);
			//3.this_thread::yield(); // == sleep_for(0ms)
		}

		// 스핀락 대기와 락 설정이 따로되어있어서 경합 발생할 수 있음 ( 옳은 스핀락 x )
		/*while (_locked)
		{

		}

		_locked = true;*/
	}

	void unlock()
	{
		//_locked = false;
		_locked.store(false); // atomic 변수이므로 알아보기 쉽게 store로 값 설정
	}

private:
	atomic<bool> _locked = false; // volatile : 컴파일러한테 최적화를 하지 말아라고 부탁하는 것 ( C++은 그냥 최적화를 하지말라는 뜻 말고는 없다 ) . atomic을 쓰면 volatile도 포함하므로 volatile은 잊자
};

int32 sum = 0;
mutex m;
SpinLock spinLock;

void Add()
{
	for (int32 i = 0; i < 10'0000; i++)
	{
		//lock_guard<mutex> guard(m);
		lock_guard<SpinLock> guard(spinLock);
		sum++;
	}
}

void Sub()
{
	for (int32 i = 0; i < 10'0000; i++)
	{
		//lock_guard<mutex> guard(m);
		lock_guard<SpinLock> guard(spinLock);
		sum--;
	}
}

int main()
{
	thread t1(Add);
	thread t2(Sub);

	t1.join();
	t2.join();

	cout << sum << endl;
}