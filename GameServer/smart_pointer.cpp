#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "RefCounting.h"

// RefCountble을 상속받으므로 이 객체는 항상 자신을 참조하는 아이의 개수를 추적할 수 있게됨
class Wraight : public RefCountable
{
public:
	int _hp = 150;
	int _posX = 0;
	int _posY = 0;
};

using WraightRef = TSharedPtr<Wraight>;

class Missile : public RefCountable
{
public:
	// 타겟 설정
	void SetTarget(WraightRef target)
	{
		_target = target;
	}

	// TODO : 타겟을 미사일이 쫓아간다 ( 무한 루프 )
	bool Update()
	{
		if (_target == nullptr)
			return true;
		 
		int posX = _target->_posX;
		int posY = _target->_posY;

		if (_target->_hp == 0)
		{
			_target = nullptr;
			return true;
		}

		return false;
	}

	WraightRef _target = nullptr;
};

using MissileRef = TSharedPtr<Missile>;

int main()
{
	WraightRef wraight(new Wraight()); 
	// new Wraight() : ReftCountable을 상속하므로 이 시점에 ref count = 1로 초기 세팅됨
	// TSharedPtr로 래핑 : new Wraight()를 관리할 포인터로 저장하고 해당 포인터 ref count 1 증가
	wraight->ReleaseRef(); // 정책상 래핑까지 되면 레퍼 카운트가 2라서 처음에만 -1해서 1로 세팅해줌

	MissileRef missile(new Missile());
	missile->ReleaseRef();

	//여기부터는  TSharedPtr로 포인터를 관리하므로 멋대로 ReleaseRef, AddRef를 호출하면 안 된다.
	//여기부터는 TSharedPtr이 레퍼 카운트를 통해 메모리 관리를 알아서 관리하기 때문!

	missile->SetTarget(wraight);

	// 레이스가 피격 당함
	wraight->_hp = 0;
	wraight = nullptr; 
	// 내부적으로 복사 생성자, 대입 연산자 다 구현했으므로 알아서 Release를 호출하여 레퍼 카운트 줄이고, 포인터도 null로 바꿔준다.

	while (true)
	{
		if (missile)
		{
			if (missile->Update())
			{
				missile->ReleaseRef();
				missile = nullptr;
			}
		}
	}

	missile = nullptr;
}