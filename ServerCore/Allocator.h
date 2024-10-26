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
	using value_type = T; // 데이터 타입

	StlAllocator() { }

	// 삭제가 될 때 다른 타입을 받아서 카피를 하려고 하는데 이걸 위해서 뭔가를 만들어 줘야 한다.
	// 그냥 만들어두고 뭐 내부적인 내용까지는 몰라도됨
	// 이게 있어야 빌드가 됨
	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) { }

	// StlAllocator를 stl 생성 시 두번째 인자로 넣으면
	// 이제 new, delete가 아닌 여기 만든 allocate, deallocate를 호출함
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