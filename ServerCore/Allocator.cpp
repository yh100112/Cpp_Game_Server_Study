#include "pch.h"
#include "Allocator.h"


/*------------
Base Allocator
------------*/
void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}

/*------------
Stomp Allocator
- 개발 단계에서 메모리 오염 버그를 잡아줄 수 있다는 큰 장점이 있음
- 단점으로는 4kb만 할당해도 4096이라는 큰 kb 를 기본적으로 할당해줌 ( 페이즈 기본단위 0x1000으로 세팅함 )
------------*/
void* StompAllocator::Alloc(int32 size)
{
	// 4 + 4095 / 4096 = 1 -> 반올림 코드
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	const int64 dataOffset = pageCount * PAGE_SIZE - size;

	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	// int8*로 baseAddress를1바이트 단위로 바꿔서 바이트 단위로 계산할 수 있게 함
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset); 
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE);
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}