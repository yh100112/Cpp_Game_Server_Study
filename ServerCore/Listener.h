#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;
class ServerService;

// 문지기가 되는 녀석
// listener를 만든 후 이 아이를 관심대상으로 리슨해라라고 해줌
class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	/*외부에서 사용*/
	bool StartAccept(ServerServiceRef service);
	void CloseSocket();

public:
	/*인터페이스 IocpObject 구현*/
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/*수신 관련 -  둘이 짝꿍 */
	void RegisterAccept(AcceptEvent* acceptEvent); // 비동기 함수를 걸어주는 역할을 함
	void ProcessAccept(AcceptEvent* acceptEvent);  // RegisterAccept()가 성공적으로 들어오면 실행

protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;
	ServerServiceRef _service;
};
