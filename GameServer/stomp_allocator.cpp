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
	// 유저레벨 ( 메모장, 롤, 서버 )
	// ---------------------
	// 커널레벨 (OS Code)

	// 운영체제가 메모리를 관리하고 할당할 때는 페이즈 단위로 관리한다.
	// ex) 메모리 : 2GB / 페이즈 : 4KB [ooooxxxxooooxxxxoooooooooooo ]
	// [r][r][w][rw][X][]...


	// 실제 메모리 할당을 운영체제에 직접 함
	SYSTEM_INFO info;
	::GetSystemInfo(&info);

	info.dwPageSize; // 페이즈 사이즈 : 4KB (0x1000)
	info.dwAllocationGranularity; // 64KB ( 0x10000 ) : 메모리를 할당할 때 이 숫자의 배수를 메모리로 할당	

	//// new와 달리 메모리 할당을 세부적으로 해줌
	//int* test = (int*)::VirtualAlloc(NULL, 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	//*test = 100;
	//::VirtualFree(test, 0, MEM_RELEASE); // 운영체제에 말해서 이 영역을 완전 날림
	//*test = 200; // crash 발생

	//// new, delete로 메모리 할당
	//// delete를 한다고 무조건 그 메모리를 날리지 않고
	//Knight* test2 = new Knight;
	//test2->_hp = 100;
	//delete test2;
	//test2->_hp = 200; 
	//// delete를 한다고 운영체제에서 그 메모리를 날리지 않고 힙 영역을 유동적으로 자기가 관리하기 떄문에 crash가 안 나고 해당 메모리가 사용된 것

	//Player* p = new Player();
	//Knight* k = static_cast<Knight*>(p); // 다운캐스팅은 static으로 안 됨
	//k->_hp = 200; // 엉뚱한 메모리를 건드는 것 ( 근데 crash 안남 ->심각한 문제 )



	// StompAllocator로 하면 딴애들은 못 잡는 메모리 오염 문제를 잡아줄 수 있다.
	// use after free 문제를 잡아줌
	// 단, 메모리 오버플로우 문제는 해결 못함
	// 우리가 필요한 건  [  ] 인데 stomp는 [              ]만큼 운영체제에서 할당해주므로
	// 필요한 [  ] 범위를 넘어가서 할당해도 하드웨어적으로는 [              ]을 할당했으므로 문제라고 여기지 않음
	//Knight* knight = xnew<Knight>(100);
	//xdelete(knight);
	//knight->_hp = 100; // crash
	Knight* knight = static_cast<Knight*>(xnew<Player>());
	knight->_hp = 100; // 여기서 메모리 오버플로우로 crash 나야 하는데 나지 않음 ( allocator에서 수정해서 이제잡음! )
	xdelete(knight);

	// stomp allocator의 오버플로우 문제 해결 방법
	// -> [                     [   ]] 필요 메모리를 할당된 메모리의 맨 끝에 할당함
}