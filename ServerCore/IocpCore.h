#pragma once

// enable_shared_from_this : �ڱ� �ڽ� �ȿ��� �ڽ��� shared_ptr�� �����ϰ� ����
// ���������� �ڱ� �ڽſ� ���� weak_ptr�� ������ ��
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
	bool Dispatch(uint32 timeoutMs = INFINITE); // IOCP�� �ϰ��� ���� ��� �����ϴ� ��

private:
	HANDLE _iocpHandle;
};
