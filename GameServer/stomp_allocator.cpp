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

class Player
{
public:
	Player(){}
	virtual ~Player(){}
};


class Knight : public Player
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

	int32 _hp = 50;
	int32 _mp = 10;
};

int main()
{
	// 가상 메모리 기본
	int* num = new int;
	*num = 100;




	Player* p = new Player();
	Knight* k = static_cast<Knight*>(p); // 다운캐스팅은 static으로 안 됨
	k->_hp = 200; // 엉뚱한 메모리를 건드는 것 ( 근데 에러가 안남 ->심각한 문제 )




	Knight* knight = xnew<Knight>(100);

	xdelete(knight);
}