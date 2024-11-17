#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	DisConnect,
	Accept,
	//PreRecv,
	Recv,
	Send
};

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);

	void			Init();

public:
	EventType		eventType;
	IocpObjectRef	owner;
};

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) { }
};

class DisConnectEvent : public IocpEvent
{
public:
	DisConnectEvent() : IocpEvent(EventType::DisConnect) { }
};


class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) { }

public:
	SessionRef session = nullptr;
};

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) { }
};

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) { }
	
	/* 
	sendQueue에서 버퍼를 빼갈 때 레퍼카운트가 줄기 때문에 
	WSASend 끝날 때까지 사라지지 않게 하기 위해 그걸 다시 한 번 여기서 보관해주는 개념
	*/
	Vector<SendBufferRef> sendBuffers; 
};

