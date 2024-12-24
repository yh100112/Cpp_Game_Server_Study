#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "RefCounting.h"

// RefCountble�� ��ӹ����Ƿ� �� ��ü�� �׻� �ڽ��� �����ϴ� ������ ������ ������ �� �ְԵ�
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
	// Ÿ�� ����
	void SetTarget(WraightRef target)
	{
		_target = target;
	}

	// TODO : Ÿ���� �̻����� �Ѿư��� ( ���� ���� )
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
	// new Wraight() : ReftCountable�� ����ϹǷ� �� ������ ref count = 1�� �ʱ� ���õ�
	// TSharedPtr�� ���� : new Wraight()�� ������ �����ͷ� �����ϰ� �ش� ������ ref count 1 ����
	wraight->ReleaseRef(); // ��å�� ���α��� �Ǹ� ���� ī��Ʈ�� 2�� ó������ -1�ؼ� 1�� ��������

	MissileRef missile(new Missile());
	missile->ReleaseRef();

	//������ʹ�  TSharedPtr�� �����͸� �����ϹǷ� �ڴ�� ReleaseRef, AddRef�� ȣ���ϸ� �� �ȴ�.
	//������ʹ� TSharedPtr�� ���� ī��Ʈ�� ���� �޸� ������ �˾Ƽ� �����ϱ� ����!

	missile->SetTarget(wraight);

	// ���̽��� �ǰ� ����
	wraight->_hp = 0;
	wraight = nullptr; 
	// ���������� ���� ������, ���� ������ �� ���������Ƿ� �˾Ƽ� Release�� ȣ���Ͽ� ���� ī��Ʈ ���̰�, �����͵� null�� �ٲ��ش�.

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