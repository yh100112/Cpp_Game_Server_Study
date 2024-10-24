#pragma once
// 할당 정책을 정의

/*------------
Base Allocator
------------*/
class BaseAllocator
{
public:
	static void* Alloc(int32 size); // 메모리 할당
	static void Release(void* ptr); // 메모리 해제
};

/*------------
Stomp Allocator
-> 유일한 기능이 버그를 잡는데 유용함
------------*/