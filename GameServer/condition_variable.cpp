#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

mutex m;
queue<int32> q;

// 참고) CV는 User-Level Object (커널 오브젝트 아님)
condition_variable cv;

void Producer()
{

	while (true)
	{
		// 1) Lock을 잡고
		// 2) 공유 변수 값을 수정
		// 3) Lock을 풀고
		// 4) 조건변수 통해 다른 스레드에게 통지
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		cv.notify_one(); // wait 중인 스레드가 있으면 딱 1개를 깨운다.
	}

}

void Consumer()
{
	while (true)
	{
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() { return q.empty() == false; });
		// 1) Lock을 잡고
		// 2) 조건 확인
		// - 만족0 => 빠져 나와서 이어서 코드를 진행
		// - 만족X => Lock을 풀어주고 대기 상태 전환

		// 그런데 notify_one을 했으면 항상 조건식을 만족하는거 아닌가?
		// Squrious Wakeup (가짜 기상?)
		// -> producer의 락 거는 곳과 notify_one은 별개의 동작이므로 그 사이에 다른 이가 껴서 가져가버리면 데이터가 없는 상황이 발생할 수 있다.

		int32 data = q.front();
		q.pop();
		cout << q.size() << endl;
	}
}

int main()
{
	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();
}