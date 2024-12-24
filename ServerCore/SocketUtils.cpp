#include "pch.h"
#include "SocketUtils.h"

/*------------------
	SocketUtils
------------------*/
LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;

// ��Ÿ�ӿ� ConnectEx, DisconnectEx, AcceptEx �Լ����� �������� �Լ�
bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;

	// ������ �ʿ� ����. �� 3�� �Լ����� ��Ÿ�ӿ� �ҷ����� �Լ���� �˸� ��
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, 
		sizeof(guid), fn, sizeof(*fn), OUT &bytes, NULL, NULL);
}

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(::WSAStartup(MAKEWORD(2, 2), OUT & wsaData) == 0); // ������ �ʱ�ȭ

	/* ��Ÿ�ӿ� �ּ� ������ API */
	SOCKET dummySocket = CreateSocket();

	// �ι�° ���� : ã�ƿ��� ���� �Լ��� �ִ´� -> ã�ƿ� �Լ��� 3��° ���ڿ� ���� �Ϳ� �־��ش�.
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));

	Close(dummySocket);
}

bool SocketUtils::Bind(SOCKET socket, NetAddress netAddr)
{
	return SOCKET_ERROR != 
		::bind(socket, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSockAddr()), sizeof(SOCKADDR_IN));
}

bool SocketUtils::BindAnyAddress(SOCKET socket, uint16 port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	return SOCKET_ERROR != 
		::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

bool SocketUtils::Listen(SOCKET socket, int32 backlog)
{
	return SOCKET_ERROR != ::listen(socket, backlog);
}

// ListenSocket�� Ư���� ClientSocket�� �״�� ������
bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

void SocketUtils::Close(SOCKET socket)
{
	if (socket != INVALID_SOCKET)
		::closesocket(socket);	
	socket = INVALID_SOCKET;
}

void SocketUtils::Clear()
{
	::WSACleanup(); // ��Ģ�� WSAStartup�� WSACleanup�� ȣ�� Ƚ���� ���ƾ� ��
}


SOCKET SocketUtils::CreateSocket()
{
	// ::socket() �̰� �ᵵ �Ǵµ� WSASocket()�� ����ϸ� �� ����ȭ �� �� �־ �̰� ���
	// ::socket()���� ���� �⺻������ �񵿱� �Լ��� ����� �� ����
	// WSA_FLAG_OVERLAPPED : overlapped�� ����ϴ� �񵿱� �Լ����� ���
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

/*
������ Linger �ɼ��� �����ϴ� �Լ�
- ������ ���� �� ��� �ð�(linger time)�� �����Ͽ� ��� �Ⱓ ���� �����ִ� �����͸� �������� ��
- �� �ɼ��� ������ close() �Ǵ� shutdown() ȣ�� ��, ���� ���۵��� ���� �����͸� ó���ϴ� ����� ����
�� ���� �Ű������� ������, ������ ���� ���� ����� �޶���
- Linger Ȱ��ȭ ����(on/off)  : Linger �ɼ��� Ȱ��ȭ���� ���θ� ����
- Linger �ð�(timeout)       : ������ ���� �� ���� �����͸� ������ ���� ����ϴ� �ð��� �� ������ ����
*/
bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 linger)
{
	LINGER option;
	option.l_onoff = onoff;
	option.l_linger = linger;
	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
}
