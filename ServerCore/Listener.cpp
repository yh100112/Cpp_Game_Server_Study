#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		// todo

		xdelete(acceptEvent);
	}
}

// 낚시 시작
bool Listener::StartAccept(ServerServiceRef service)
{
	_service = service;
	if (_service == nullptr)
		return false;

	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	if (_service->GetIocpCore()->Register(shared_from_this()) == false) // _socket을 반환
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false) // 이거 안하면 종종 주소가 겹치는 문제로 서버 안 뜸
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketUtils::Bind(_socket, _service->GetNetAddress()) == false)
		return false;
	
	if (SocketUtils::Listen(_socket) == false)
		return false;

	const int32 acceptCount = _service->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; i++)
	{
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		acceptEvent->owner = shared_from_this(); // 자기 자신에 대한 weak_ptr을 통해 shared_ptr을 추출해줌
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	ASSERT_CRASH(iocpEvent->eventType == EventType::Accept);

	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent); // 복원
	ProcessAccept(acceptEvent);
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = acceptEvent->session;

	// ListenSocket(_socket)의 특성을 ClientSocket(session->GetSocket())에 그대로 적용함
	if (SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket) == false)
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == 
		::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}
	
	// 여기까지 왔다는 건 세션이 성공적으로 접속되어 커넥션이 맺어짐
	session->SetNetAddress(NetAddress(sockAddress));
	//cout << "Client Connected!" << endl;

	// 여기서 session의 함수를 호출해줌
	session->ProcessConnect();

	RegisterAccept(acceptEvent);
}

// accept를 해줌
// 누군가가 connect를 하면 iocp가 dispatch()를 통해 인지한다.
void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = _service->CreateSession(); // IOCP에 해당 세션 등록
	
	acceptEvent->Init();
	acceptEvent->session = session;
	
	DWORD bytesReceived = 0;
	// 비동기 accept를 걸어주는 부분
	if (SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer.WritePos(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)) == false)
	{
		const int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) // 팬딩이 아닌 경우에 여기 들어오면 다시 accept 걸어줌 - 낚시대 던지는 행위
		{
			// 일단 다시 Accept 걸어준다
			RegisterAccept(acceptEvent);
		}
	}
}

