#include "pch.h"
#include "Allocator.h"
#include "Memory.h"


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

	// 할당할 메모리의 offset을 page의 맨 끝 부분에 할당 하도록 함 - memory overflow를 잡아주기 위함
	const int64 dataOffset = pageCount * PAGE_SIZE - size; 

	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	// int8*로 baseAddress를 바이트 단위로 바꿔서 바이트 단위로 dataOffset과 계산을 할 수 있게 함
	// 포인터 계산은 포인터 크기에 따라 영향을 주므로 확실하게 바이트 단위로 캐스팅한 후 dataOffset과 더한 거임
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset); 
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE);
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}

/*------------
Pool Allocator
------------*/
void* PoolAllocator::Alloc(int32 size)
{
	return GMemory->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	GMemory->Release(ptr);
}