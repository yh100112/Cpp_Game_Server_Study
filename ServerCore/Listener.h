#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

// 문지기가 되는 녀석
// listener를 만든 후 이 아이를 관심대상으로 리슨해라라고 해줌
class Listener : public IocpObject
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

