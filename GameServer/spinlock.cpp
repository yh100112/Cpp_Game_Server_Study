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
		bool expected = false; // �ʱⰪ
		bool desired = true; // �ʱⰪ

		// CAS �ǻ��ڵ�
		//if (_locked == expected) // lock�� false��
		//{
		//	expected = _locked;
		//	_locked = desired; // lock�� true�� �ٲ���
		//	return true; // lock�� ���������ڸ��� ���ɶ� ��� ��������
		//}
		//else
		//{
		//	expected = _locked;
		//	return false;
		//}

		// CAS ( Compare-And-Swap ) : ���ɶ� ���� �� �������ִ°� atomic�ϰ� �ѹ��� ���ֱ� ���� �ʿ� (���� ���ɶ�)
		while (_locked.compare_exchange_strong(expected, desired) == false)
		{
			expected = false; // CAS ���� �Ź� expected�� locked�� ���� ���ŵǹǷ� �ʱⰪ�� false�� �ٽ� ��������� ��

			// sleep ��� ( ���� ��Ÿ ) - Ŀ�� ���� ���� ��  (system call - os���� ��û�ϴ� �κ�)
			//1. this_thread::sleep_for(std::chrono::milliseconds(100));
			//2. this_thread::sleep_for(100ms);
			//3.this_thread::yield(); // == sleep_for(0ms)
		}

		// ���ɶ� ���� �� ������ ���εǾ��־ ���� �߻��� �� ���� ( ���� ���ɶ� x )
		/*while (_locked)
		{

		}

		_locked = true;*/
	}

	void unlock()
	{
		//_locked = false;
		_locked.store(false); // atomic �����̹Ƿ� �˾ƺ��� ���� store�� �� ����
	}

private:
	atomic<bool> _locked = false; // volatile : �����Ϸ����� ����ȭ�� ���� ���ƶ�� ��Ź�ϴ� �� ( C++�� �׳� ����ȭ�� ��������� �� ����� ���� ) . atomic�� ���� volatile�� �����ϹǷ� volatile�� ����
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