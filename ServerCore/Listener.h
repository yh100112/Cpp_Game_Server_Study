#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;

// 문지기가 되는 녀석
// listener를 만든 후 이 아이를 관심대상으로 리슨해라라고 해줌
class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	/*외부에서 사용*/
	bool StartAccept(NetAddress netAAddress);
	void CloseSocket();

public:
	/*인터페이스 IocpObject 구현*/
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/*수신 관련*/
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;
};
