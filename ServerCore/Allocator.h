#pragma once
// �Ҵ� ��å�� ����

/*------------
Base Allocator
------------*/
class BaseAllocator
{
public:
	static void* Alloc(int32 size); // �޸� �Ҵ�
	static void Release(void* ptr); // �޸� ����
};

/*------------
Stomp Allocator
-> ������ ����� ���׸� ��µ� ������
------------*/
class StompAllocator
{
	enum { PAGE_SIZE = 0x1000 };
public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};

/*------------
PoolAllocator
------------*/
class PoolAllocator
{
public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};

/*------------
	STL Allocator
------------*/
template<typename T>
class StlAllocator
{
public:
	using value_type = T; // ������ Ÿ��

	StlAllocator() { }

	// ������ �� �� �ٸ� Ÿ���� �޾Ƽ� ī�Ǹ� �Ϸ��� �ϴµ� �̰� ���ؼ� ������ ����� ��� �Ѵ�.
	// �׳� �����ΰ� �� �������� ��������� ���󵵵�
	// �̰� �־�� ���尡 ��
	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) { }

	// StlAllocator�� stl ���� �� �ι�° ���ڷ� ������
	// ���� new, delete�� �ƴ� ���� ���� allocate, deallocate�� ȣ����
	T* allocate(size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(Xalloc(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		Xrelease(ptr);
	}

};