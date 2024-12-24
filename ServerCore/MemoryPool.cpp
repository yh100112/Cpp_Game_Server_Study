#include "pch.h"
#include "MemoryPool.h"

/*---------------
	MemoryPool
----------------*/

MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize)
{
	::InitializeSListHead(&_header);
}

MemoryPool::~MemoryPool()
{
	//while (_queue.empty() == false)
	//{
	//	MemoryHeader* header = _queue.front();
	//	_queue.pop();
	//	::free(header);
	//}
	
	// memory�� ���� �ƴϸ� while ������ ��
	while (MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header)))
		::_aligned_free(memory);
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	//WRITE_LOCK;
	ptr->allocSize = 0;

	// Pool�� �޸� �ݳ�
	//_queue.push(ptr);
	::InterlockedPushEntrySList(&_header, static_cast<PSLIST_ENTRY>(ptr));

	_allocCount.fetch_sub(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header));
	//MemoryHeader* header = nullptr;
	//{
	//	WRITE_LOCK;
	//	// Pool�� ������ �ִ���?
	//	if (_queue.empty() == false)
	//	{
	//		// ������ �ϳ� ������
	//		header = _queue.front();
	//		_queue.pop();
	//	}
	//}

	// ������ ���� �����
	if (memory == nullptr)
	{
		//header = reinterpret_cast<MemoryHeader*>(::malloc(_allocSize));
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(_allocSize, SLIST_ALLIGNMENT));
	}
	else
	{
		ASSERT_CRASH(memory->allocSize == 0); // 0�� �ƴϸ� crash �߻���Ŵ
	}

	_allocCount.fetch_add(1);

	return memory;
}
