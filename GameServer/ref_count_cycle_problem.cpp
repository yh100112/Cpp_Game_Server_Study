#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "RefCounting.h"


class Knight
{
public:
	Knight()
	{
		cout << "Knight()" << endl;
	}

	~Knight()
	{
		cout << "~Knight()" << endl;
	}

	void SetTarget(KnightRef target)
	{
		_target = target;
	}
};

int main()
{
	// 순환 문제 ( cycle )

	// unique_ptr<Knight> k2 = make_unique<Knight>(); // 생 포인터랑 거의 흡사. 부하도 거의없음. 단, 복사가 안 됨
	
	// waek_ptr : 사이클을 해결해줌

	// [T*][RefCountBlocking*] - 공간 2개
	//shared_ptr<Knight> spr(new Knight());

	// [T* | RefCountingBlock*] - 공간 1개
	// RefCountBlock(useCount(shared), weakCount(weak))
	shared_ptr<Knight> spr = make_shared<Knight>();
	weak_ptr<Knight> wpr = spr; 
	// ref count block을 참조해서 그 객체가 사라졌는지는 알수있지만 
	// knight의 레퍼 카운팅 수명에는 영향이 없는 카운트
	// - shared와 weak는 서로 상호보완해줌

	// weak_ptr이 자원을 할당받는 방법
	bool expired = wpr.expired();
	shared_ptr<Knight> spr2 = wpr.lock();


}