#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

// �����Ⱑ �Ǵ� �༮
// listener�� ���� �� �� ���̸� ���ɴ������ �����ض��� ����
class Listener : public IocpObject
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

