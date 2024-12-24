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
	// �������� ( �޸���, ��, ���� )
	// ---------------------
	// Ŀ�η��� (OS Code)

	// �ü���� �޸𸮸� �����ϰ� �Ҵ��� ���� ������ ������ �����Ѵ�.
	// ex) �޸� : 2GB / ������ : 4KB [ooooxxxxooooxxxxoooooooooooo ]
	// [r][r][w][rw][X][]...


	// ���� �޸� �Ҵ��� �ü���� ���� ��
	SYSTEM_INFO info;
	::GetSystemInfo(&info);

	info.dwPageSize; // ������ ������ : 4KB (0x1000)
	info.dwAllocationGranularity; // 64KB ( 0x10000 ) : �޸𸮸� �Ҵ��� �� �� ������ ����� �޸𸮷� �Ҵ�	

	//// new�� �޸� �޸� �Ҵ��� ���������� ����
	//int* test = (int*)::VirtualAlloc(NULL, 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	//*test = 100;
	//::VirtualFree(test, 0, MEM_RELEASE); // �ü���� ���ؼ� �� ������ ���� ����
	//*test = 200; // crash �߻�

	//// new, delete�� �޸� �Ҵ�
	//// delete�� �Ѵٰ� ������ �� �޸𸮸� ������ �ʰ�
	//Knight* test2 = new Knight;
	//test2->_hp = 100;
	//delete test2;
	//test2->_hp = 200; 
	//// delete�� �Ѵٰ� �ü������ �� �޸𸮸� ������ �ʰ� �� ������ ���������� �ڱⰡ �����ϱ� ������ crash�� �� ���� �ش� �޸𸮰� ���� ��

	//Player* p = new Player();
	//Knight* k = static_cast<Knight*>(p); // �ٿ�ĳ������ static���� �� ��
	//k->_hp = 200; // ������ �޸𸮸� �ǵ�� �� ( �ٵ� crash �ȳ� ->�ɰ��� ���� )



	// StompAllocator�� �ϸ� ���ֵ��� �� ��� �޸� ���� ������ ����� �� �ִ�.
	// use after free ������ �����
	// ��, �޸� �����÷ο� ������ �ذ� ����
	// �츮�� �ʿ��� ��  [  ] �ε� stomp�� [              ]��ŭ �ü������ �Ҵ����ֹǷ�
	// �ʿ��� [  ] ������ �Ѿ�� �Ҵ��ص� �ϵ���������δ� [              ]�� �Ҵ������Ƿ� ������� ������ ����
	//Knight* knight = xnew<Knight>(100);
	//xdelete(knight);
	//knight->_hp = 100; // crash
	Knight* knight = static_cast<Knight*>(xnew<Player>());
	knight->_hp = 100; // ���⼭ �޸� �����÷ο�� crash ���� �ϴµ� ���� ���� ( allocator���� �����ؼ� ��������! )
	xdelete(knight);

	// stomp allocator�� �����÷ο� ���� �ذ� ���
	// -> [                     [   ]] �ʿ� �޸𸮸� �Ҵ�� �޸��� �� ���� �Ҵ���
}