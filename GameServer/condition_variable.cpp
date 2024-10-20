#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

mutex m;
queue<int32> q;

// ����) CV�� User-Level Object (Ŀ�� ������Ʈ �ƴ�)
condition_variable cv;

void Producer()
{

	while (true)
	{
		// 1) Lock�� ���
		// 2) ���� ���� ���� ����
		// 3) Lock�� Ǯ��
		// 4) ���Ǻ��� ���� �ٸ� �����忡�� ����
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		cv.notify_one(); // wait ���� �����尡 ������ �� 1���� �����.
	}

}

void Consumer()
{
	while (true)
	{
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() { return q.empty() == false; });
		// 1) Lock�� ���
		// 2) ���� Ȯ��
		// - ����0 => ���� ���ͼ� �̾ �ڵ带 ����
		// - ����X => Lock�� Ǯ���ְ� ��� ���� ��ȯ

		// �׷��� notify_one�� ������ �׻� ���ǽ��� �����ϴ°� �ƴѰ�?
		// Squrious Wakeup (��¥ ���?)
		// -> producer�� �� �Ŵ� ���� notify_one�� ������ �����̹Ƿ� �� ���̿� �ٸ� �̰� ���� ������������ �����Ͱ� ���� ��Ȳ�� �߻��� �� �ִ�.

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