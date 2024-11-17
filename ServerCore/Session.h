#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

class Service;


/*-------------
	Session
-------------*/
class Session : public IocpObject
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

	enum
	{
		BUFFER_SIZE = 0x10000, // 64KB

	};

public:
	Session();
	virtual ~Session();

public:
	/*외부에서 사용*/	
	void				Send(SendBufferRef sendBuffer);
	bool				Connect();
	void				Disconnect(const WCHAR* cause);

	shared_ptr<Service>	GetService() { return _service.lock(); }
	void				SetService(shared_ptr<Service> service) { _service = service; }

public:
	/*정보 관련*/
	void				SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress			GetAddress() { return _netAddress; }
	SOCKET				GetSocket() { return _socket; }
	bool				IsConnected() { return _connected; }
	SessionRef			GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

private:
	/*인터페이스 구현*/
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/* 전송 관련 */
	bool				RegisterConnect(); // Register - Process 둘이 짝꿍
	bool				RegisterDisconnect();
	void				RegisterRecv();
	void				RegisterSend();

	void				ProcessConnect();
	void				ProcessDisConnect();
	void				ProcessRecv(int32 numOfBytes);
	void				ProcessSend(int32 numOfBytes);

	void				HandleError(int32 errorCode);

protected:
	/*컨텐츠 코드에서 오버로딩*/
	virtual void		OnConnected() { }
	virtual int32		OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void		OnSend(int32 len) { }
	virtual void		OnDisconnected() { }

public:
	// circular buffer
	//char _sendBuffer[1000];
	//int32 _sendLen = 0;

private:
	weak_ptr<Service>	_service; // 내부에서 서비스에 대한 존재를 알아야 서비스에 자신을 등록하거나 끊을 수 있으므로 내부에 들고 있어야 함
	SOCKET				_socket = INVALID_SOCKET;
	NetAddress			_netAddress = {};
	Atomic<bool>		_connected = false;

private:
	USE_LOCK;

	/*수신 관련*/
	RecvBuffer				_recvBuffer;

	/*송신 관련*/
	Queue<SendBufferRef>	_sendQueue; // send할 작업들을 넣어줌
	Atomic<bool>			_sendRegistered = false;

private:
	/*IocpEvent 재사용*/	
	ConnectEvent		_connectEvent;
	DisConnectEvent		_disconnectEvent;
	RecvEvent			_recvEvent;
	SendEvent			_sendEvent;
};

/*-------------
	PacketSession
-------------*/
// [size(2byte)][id(2byte)][data.....][size(2byte)][id(2byte)][data.....]
struct PacketHeader
{
	uint16 size;
	uint16 id; // 프로토콜 ID (ex. 1 = 로그인, 2 = 이동 요청)
};

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

	PacketSessionRef GetpacketSessionRef() { return static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int32		OnRecv(BYTE* buffer, int32 len) sealed; // OnRecv는 PacketSession에서 끊긴다
	virtual void		OnRecvPacket(BYTE* buffer, int32 len) abstract;
};
