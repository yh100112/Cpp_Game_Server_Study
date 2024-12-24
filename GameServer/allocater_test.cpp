#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"
#include "RefCounting.h"
#include "Memory.h"

class Knight
{
public:
	Knight()
	{
		cout << "Knight()" << endl;
	}

	Knight(int32 hp) : _hp(hp)
	{
		cout << "Knight(hp)" << endl;
	}

	~Knight()
	{
		cout << "~Knight()" << endl;
	}

	//// new�� ���� ������ �־ �̰� ���ܵθ� Memory.h�� ���� xnew�� �浹�� �Ͼ �ּ�ó��
	//// operator new�� delete�� Ư���� �༮���̶� �̷��Ը� �ᵵ Ŭ���� ���ο� �����ϸ� static�� ���� �Ͱ� ����.
	//void* operator new(size_t size)
	//{
	//	cout << "Knight new" << size << endl;
	//	void* ptr = ::malloc(size);
	//	return ptr;
	//}

	//void operator delete(void* ptr)
	//{
	//	cout << "Knight delete!" << endl;
	//	::free(ptr);
	//}

	int32 _hp = 50;
	int32 _mp = 10;
};

int main()
{
	Knight* knight = xnew<Knight>(100);

	xdelete(knight);
}