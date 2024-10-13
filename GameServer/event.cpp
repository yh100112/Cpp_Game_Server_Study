#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h> // window api ����ؼ� �̺�Ʈ ����

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

		::SetEvent(handle); // Ŀ�� ������Ʈ�� �ñ׳� ���¸�  �ٲ��ּ���.
		this_thread::sleep_for(10000ms);
	}

}

void Consumer()
{
	while (true)
	{
		::WaitForSingleObject(handle, INFINITE); // Ŀ�� ������Ʈ�� �ñ׳� ���¸� ���� �Ķ����̸� ��� ����, �������̶�� �� �����尡 �Ͼ�� �ʰ� ���� ���·� ���� �ְ� �ȴ�. ( �ῡ�� �� ������ cpu�� ��Ƹ��� �ʴ´�. )
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
	// Ŀ�� ������Ʈ ( Ŀ�ο��� ����ϴ� ������Ʈ )
	// Usage Count
	// Signal (�Ķ���), Non-Signal (������) >> bool
	// ManualReset << bool : �̰� true�� �ϸ� waitforsingleobject������ ������ signal�� ����� �Ѵ�. false�� �˾Ƽ� signal�� ��� �� �ٷ� ���ش�.
	handle = ::CreateEvent(NULL/*���ȼӼ�*/, FALSE/*bManualReset*/, FALSE/*binitialState*/, NULL); // windowAPI - Ŀ�� ������Ʈ

	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();

	::CloseHandle(handle);
}