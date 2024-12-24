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
	// ��ȯ ���� ( cycle )

	// unique_ptr<Knight> k2 = make_unique<Knight>(); // �� �����Ͷ� ���� ���. ���ϵ� ���Ǿ���. ��, ���簡 �� ��
	
	// waek_ptr : ����Ŭ�� �ذ�����

	// [T*][RefCountBlocking*] - ���� 2��
	//shared_ptr<Knight> spr(new Knight());

	// [T* | RefCountingBlock*] - ���� 1��
	// RefCountBlock(useCount(shared), weakCount(weak))
	shared_ptr<Knight> spr = make_shared<Knight>();
	weak_ptr<Knight> wpr = spr; 
	// ref count block�� �����ؼ� �� ��ü�� ����������� �˼������� 
	// knight�� ���� ī���� ������ ������ ���� ī��Ʈ
	// - shared�� weak�� ���� ��ȣ��������

	// weak_ptr�� �ڿ��� �Ҵ�޴� ���
	bool expired = wpr.expired();
	shared_ptr<Knight> spr2 = wpr.lock();


}