#include "pch.h"
#include "SocketUtils.h"

/*------------------
	SocketUtils
------------------*/
LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;

// 런타임에 ConnectEx, DisconnectEx, AcceptEx 함수들을 가져오는 함수
bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;

	// 이해할 필요 없다. 위 3개 함수들을 런타임에 불러오는 함수라고만 알면 됨
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, 
		sizeof(guid), fn, sizeof(*fn), OUT &bytes, NULL, NULL);
}

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(::WSAStartup(MAKEWORD(2, 2), OUT & wsaData) == 0); // 윈속을 초기화

	/* 런타임에 주소 얻어오는 API */
	SOCKET dummySocket = CreateSocket();

	// 두번째 인자 : 찾아오고 싶은 함수를 넣는다 -> 찾아온 함수를 3번째 인자에 적힌 것에 넣어준다.
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

// ListenSocket의 특성을 ClientSocket에 그대로 적용함
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
	::WSACleanup(); // 원칙상 WSAStartup과 WSACleanup의 호출 횟수가 같아야 함
}


SOCKET SocketUtils::CreateSocket()
{
	// ::socket() 이걸 써도 되는데 WSASocket()을 사용하면 더 세분화 할 수 있어서 이걸 사용
	// ::socket()으로 만들어도 기본적으로 비동기 함수를 사용할 수 있음
	// WSA_FLAG_OVERLAPPED : overlapped를 사용하는 비동기 함수들을 사용
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

/*
소켓의 Linger 옵션을 설정하는 함수
- 소켓이 닫힐 때 대기 시간(linger time)을 설정하여 대기 기간 동안 남아있는 데이터를 보내도록 함
- 이 옵션은 소켓의 close() 또는 shutdown() 호출 후, 아직 전송되지 않은 데이터를 처리하는 방식을 제어
두 개의 매개변수를 가지며, 설정에 따라 동작 방식이 달라짐
- Linger 활성화 여부(on/off)  : Linger 옵션을 활성화할지 여부를 결정
- Linger 시간(timeout)       : 소켓이 닫힐 때 남은 데이터를 보내기 위해 대기하는 시간을 초 단위로 설정
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
