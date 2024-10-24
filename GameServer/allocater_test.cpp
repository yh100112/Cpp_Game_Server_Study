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

	//// new는 여러 버전이 있어서 이걸 남겨두면 Memory.h에 만든 xnew와 충돌이 일어나 주석처리
	//// operator new와 delete는 특별한 녀석들이라 이렇게만 써도 클래스 내부에 선언하면 static이 붙은 것과 같다.
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