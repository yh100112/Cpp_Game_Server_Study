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

		// TODO : �Ѿư���

		if (_target->_hp == 0)
		{
			//_target->ReleaseRef(); //  ���� ī��Ʈ 1 �ٿ��ִ� ����
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
	wraight->ReleaseRef(); // ��å�� 2�� �ʱ⿡�� -1�ؼ� 1�� ��������
	MissileRef missile(new Missile());
	missile->ReleaseRef(); // ��å�� 2�� �ʱ⿡�� -1�ؼ� 1�� ��������

	missile->SetTarget(wraight);

	// ���̽��� �ǰ� ����
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