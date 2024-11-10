#pragma once

// enable_shared_from_this : 자기 자신 안에서 자신의 shared_ptr을 추출하게 해줌
// 내부적으로 자기 자신에 대한 weak_ptr를 가지게 됨
class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandle; }

	bool Register(IocpObjectRef iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE); // IOCP에 일감이 없나 계속 감지하는 것

private:
	HANDLE _iocpHandle;
};
