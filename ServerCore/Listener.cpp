#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		// todo

		xdelete(acceptEvent);
	}
}

// ���� ����
bool Listener::StartAccept(NetAddress netAddress)
{
	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	if (GlocpCore.Register(this) == false) // _socket�� ��ȯ
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false) // �̰� ���ϸ� ���� �ּҰ� ��ġ�� ������ ���� �� ��
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketUtils::Bind(_socket, netAddress) == false)
		return false;
	
	if (SocketUtils::Listen(_socket) == false)
		return false;

	const int32 acceptCount = 1;
	for (int32 i = 0; i < acceptCount; i++)
	{
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return false;
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
	ASSERT_CRASH(iocpEvent->GetType() == EventType::Accept);

	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent); // ����
	ProcessAccept(acceptEvent);
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	Session* session = acceptEvent->GetSession();

	// ListenSocket(_socket)�� Ư���� ClientSocket(session->GetSocket())�� �״�� ������
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
	
	session->SetNetAddress(NetAddress(sockAddress));

	cout << "Client Connected!" << endl;

	// TODO

	RegisterAccept(acceptEvent);
}

// accept�� ����
// �������� connect�� �ϸ� iocp�� dispatch()�� ���� �����Ѵ�.
void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	Session* session = xnew<Session>();
	
	acceptEvent->Init();
	acceptEvent->SetSession(session);
	
	DWORD bytesReceived = 0;
	// �񵿱� accept�� �ɾ��ִ� �κ�
	if (SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer, 
		0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT &bytesReceived, 
		static_cast<LPOVERLAPPED>(acceptEvent)) == false)
	{
		const int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) // �ҵ��� �ƴ� ��쿡 ���� ������ �ٽ� accept �ɾ��� - ���ô� ������ ����
		{
			// �ϴ� �ٽ� Accept �ɾ��ش�
			RegisterAccept(acceptEvent);
		}
	}
}

