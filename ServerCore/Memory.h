#pragma once
#include "Allocator.h"

class MemoryPool;

/*-----------------
	Memory
------------------*/
class Memory
{
	enum
	{		
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256), // ~1024���� 32������ Ǯ ����, ~2048���� 128����, ~4096����Ʈ���� 256����
		MAX_ALLOC_SIZE = 4096									// 4Ű�ι���Ʈ���� �޸� Ǯ�� ����� ��
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);

private:
	vector<MemoryPool*> _pools;

	// �޸� ũ�� <-> �޸� Ǯ
	// O(1) �ȿ� ������ ã�� ���� Ǯ ���̺�
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};


// allocator.h(class BaseAllocator)���� ������ new�� �ٸ� ������� new�� ����
// ... : ���� ������ ��������
template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(Xalloc(sizeof(Type)));

	// �޸𸮰� ���� �Ҵ�� ���¿��� �� �޸� ���� � ��ü�� �����ڸ� ȣ���ϴ� ����
	// placement new��� ����
	// - �޸𸮴� �̹� �����ϱ� �޸𸮸� ���� �������� ���� �� �޸� ���� �����ڸ� ȣ������~
	// - ���� �������� ������ ���� args�� �����ڿ� �Ѱ��� ( forward<Args>() )
	new(memory)Type(std::forward<Args>(args)...);

	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	Xrelease(obj);
}
