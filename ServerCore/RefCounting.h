#pragma once

/*---------------
   RefCountable
----------------*/
class RefCountable
{
public:
	// ref count�� ��å�� �ʱⰪ�� 1�� ��
	RefCountable() : _refCount(1) { }
	
	// �޸� ���� �����ϱ� ���� �ֻ��� Ŭ������ �Ҹ��ڿ��� virtual�� �����ؾ� ��
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

	// _ptr�� ���۷��� ī������ ������ �����͸� �����԰� ���ÿ� ���۷��� ī������ ������
	TSharedPtr(T* ptr) { Set(ptr); } 

	// ���� ������
	TSharedPtr(const TSharedPtr& rhs) { Set(rhs._ptr); }
	
	// �̵� ������
	TSharedPtr(TSharedPtr&& rhs) 
	{ 
		_ptr = rhs._ptr; 
		rhs._ptr = nullptr; 
	}
	
	// ��� ���� ���� ������ (�ڽ� �����͸� �θ� �����ͷ� ��ĳ�����Ͽ� ����)
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs) { Set(static_cast<T*>(rhs._ptr)); }

	// �����Ͱ� �Ҹ�� ��  Release() �� ȣ���� ���۷��� ī��Ʈ�� 1�ٿ���
	~TSharedPtr() { Release(); }

public:
	// ���� ���� ������
	TSharedPtr& operator=(const TSharedPtr& rhs)
	{
		if (_ptr != rhs._ptr)
		{
			Release(); // ������ �Ҵ�� �����͸� �������� ( ī���� ���� )
			Set(rhs._ptr);
		}
		return *this;
	}

	// �̵� ������
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
	// ���۷��� ī������ �����͸� �����ؼ� ��� ���ִ� �Լ�
	inline void Set(T* ptr)
	{
		_ptr = ptr;
		if (ptr)
			ptr->AddRef(); // ī���� ����
	}

	// ���۷��� ī���� �ݳ��� �����͸� �����ؼ� ��� ���ִ� �Լ�
	inline void Release()
	{
		if (_ptr != nullptr)
		{
			_ptr->ReleaseRef();
			_ptr = nullptr;
		}
	}

private:
	T* _ptr = nullptr; // reference counting�� ������ �ǰ� �ִ� ������
};