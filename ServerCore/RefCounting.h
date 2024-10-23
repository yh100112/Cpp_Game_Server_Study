#pragma once

/*---------------
   RefCountable
----------------*/
class RefCountable
{
public:
	// ref count는 정책상 초기값은 1로 함
	RefCountable() : _refCount(1) { }
	
	// 메모리 릭을 예방하기 위해 최상위 클래스에 소멸자에는 virtual을 선언해야 함
	virtual ~RefCountable() { }

	int32 GetRefCount() { return _refCount; }
	int32 AddRef() { return ++_refCount; }
	int32 ReleaseRef() 
	{
		int32 refCount = --_refCount;
		if (refCount == 0)
			delete this;
		return refCount;
	}

protected:
	atomic<int32> _refCount;
};

/*---------------
   SharedPtr
----------------*/
template<typename T>
class TSharedPtr
{
public:
	TSharedPtr() {}

	// _ptr에 레퍼런스 카운팅을 관리할 포인터를 저장함과 동시에 레퍼런스 카운팅을 위임함
	TSharedPtr(T* ptr) { Set(ptr); } 

	// 복사 생성자
	TSharedPtr(const TSharedPtr& rhs) { Set(rhs._ptr); }
	
	// 이동 생성자
	TSharedPtr(TSharedPtr&& rhs) 
	{ 
		_ptr = rhs._ptr; 
		rhs._ptr = nullptr; 
	}
	
	// 상속 관계 복사 생성자 (자식 포인터를 부모 포인터로 업캐스팅하여 복사)
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs) { Set(static_cast<T*>(rhs._ptr)); }

	// 포인터가 소멸될 때  Release() 를 호출해 레퍼런스 카운트를 1줄여줌
	~TSharedPtr() { Release(); }

public:
	// 복사 대입 연산자
	TSharedPtr& operator=(const TSharedPtr& rhs)
	{
		if (_ptr != rhs._ptr)
		{
			Release(); // 이전에 할당된 포인터를 해제해줌 ( 카운팅 감소 )
			Set(rhs._ptr);
		}
		return *this;
	}

	// 이동 연산자
	TSharedPtr& operator=(TSharedPtr&& rhs)
	{
		Release();
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
		return *this;
	}

	bool operator==(const TSharedPtr& rhs) const { return _ptr == rhs._ptr; }
	bool operator==(T* ptr) const { return _ptr == ptr; }
	bool operator!=(const TSharedPtr& rhs) const { return _ptr != rhs._ptr; }
	bool operator!=(T* ptr) const { return _ptr != ptr; }
	bool operator<(const TSharedPtr& rhs) const { return _ptr < rhs._ptr; }
	T*			operator*() { return _ptr; }
	const T*	operator*() const { return _ptr; }
	operator T* () const { return _ptr; }
	T*			operator->() { return _ptr; }
	const T*	operator->() const { return _ptr; }

	bool IsNull() { return _ptr == nullptr; }

private:
	// 레퍼런스 카운팅을 포인터를 래핑해서 대신 해주는 함수
	inline void Set(T* ptr)
	{
		_ptr = ptr;
		if (ptr)
			ptr->AddRef(); // 카운팅 증가
	}

	// 래퍼런스 카운팅 반납을 포인터를 래핑해서 대신 해주는 함수
	inline void Release()
	{
		if (_ptr != nullptr)
		{
			_ptr->ReleaseRef();
			_ptr = nullptr;
		}
	}

private:
	T* _ptr = nullptr; // reference counting이 지연이 되고 있는 포인터
};