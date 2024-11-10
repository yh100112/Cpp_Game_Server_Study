#pragma once

class IocpObject
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

	bool Register(class IocpObject* iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE); // IOCP에 일감이 없나 계속 감지하는 것

private:
	HANDLE _iocpHandle;
};

// TEMP
extern IocpCore GlocpCore;
