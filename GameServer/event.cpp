#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h> // window api 사용해서 이벤트 구현

mutex m;
queue<int32> q;
HANDLE handle;

void Producer()
{

	while (true)
	{
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		::SetEvent(handle); // 커널 오브젝트의 시그널 상태를  바꿔주세요.
		this_thread::sleep_for(10000ms);
	}

}

void Consumer()
{
	while (true)
	{
		::WaitForSingleObject(handle, INFINITE); // 커널 오브젝트의 시그널 상태를 보고 파란불이면 계속 진행, 빨간불이라면 이 스레드가 일어나지 않고 수면 상태로 잠들어 있게 된다. ( 잠에서 깰 때까지 cpu를 잡아먹지 않는다. )
		//::ResetHandle(handle);

		unique_lock<mutex> lock(m);
		if (q.empty() == false)
		{
			int32 data = q.front();
			q.pop();
			cout << data << endl;
		}
	}
}

int main()
{
	// 커널 오브젝트 ( 커널에서 사용하는 오브젝트 )
	// Usage Count
	// Signal (파란불), Non-Signal (빨간불) >> bool
	// ManualReset << bool : 이걸 true로 하면 waitforsingleobject다음에 강제로 signal을 꺼줘야 한다. false면 알아서 signal을 통과 후 바로 꺼준다.
	handle = ::CreateEvent(NULL/*보안속성*/, FALSE/*bManualReset*/, FALSE/*binitialState*/, NULL); // windowAPI - 커널 오브젝트

	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();

	::CloseHandle(handle);
}