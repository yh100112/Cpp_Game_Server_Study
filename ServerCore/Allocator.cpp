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
- ���� �ܰ迡�� �޸� ���� ���׸� ����� �� �ִٴ� ū ������ ����
- �������δ� 4kb�� �Ҵ��ص� 4096�̶�� ū kb �� �⺻������ �Ҵ����� ( ������ �⺻���� 0x1000���� ������ )
------------*/
void* StompAllocator::Alloc(int32 size)
{
	// 4 + 4095 / 4096 = 1 -> �ݿø� �ڵ�
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	const int64 dataOffset = pageCount * PAGE_SIZE - size;

	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	// int8*�� baseAddress��1����Ʈ ������ �ٲ㼭 ����Ʈ ������ ����� �� �ְ� ��
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset); 
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE);
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}