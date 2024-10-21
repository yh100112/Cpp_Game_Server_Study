#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "RefCountable.h"

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
	void SetTarget(WraightRef target)
	{
		_target = target;
		//target->AddRef();
	}

	bool Update()
	{
		if (_target == nullptr)
			return true;
		 
		int posX = _target->_posX;
		int posY = _target->_posY;

		// TODO : 쫓아간다

		if (_target->_hp == 0)
		{
			//_target->ReleaseRef(); //  레퍼 카운트 1 줄여주는 동작
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
	wraight->ReleaseRef(); // 정책상 2라서 초기에만 -1해서 1로 세팅해줌
	MissileRef missile(new Missile());
	missile->ReleaseRef(); // 정책상 2라서 초기에만 -1해서 1로 세팅해줌

	missile->SetTarget(wraight);

	// 레이스가 피격 당함
	wraight->_hp = 0;
	wraight = nullptr;

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