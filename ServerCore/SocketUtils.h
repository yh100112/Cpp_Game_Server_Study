#pragma once
#include "NetAddress.h"

/*------------------
	SocketUtils
------------------*/
// 호출하는 함수들을 이렇게 래핑해서 사용하는게 좋다.
// 꼭 필요한 인자들만 사용할 때 넣어주기 때문에 (안에서 래핑하므로) 가독성이나 사용성이 좋게 된다.
class SocketUtils
{
public:
	static LPFN_CONNECTEX		ConnectEx; // connectex라는 함수의 포인터가 됨
	static LPFN_DISCONNECTEX	DisconnectEx;
	static LPFN_ACCEPTEX		AcceptEx;

public:
	static void Init();
	static void Clear();

	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET CreateSocket(); // 임의의 tcp 소켓을 만드는 함수 ( 소켓 만드는 일이 종종 생기므로 만듬 )

	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
	static bool SetReuseAddress(SOCKET socket, bool flag);
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	static bool SetSendBufferSize(SOCKET socket, int32 size);
	static bool SetTcpNoDelay(SOCKET socket, bool flag);
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

	static bool Bind(SOCKET socket, NetAddress netAddr); // socket에 특정 ip를 mapping하는 함수
	static bool BindAnyAddress(SOCKET socket, uint16 port); // socket에 임의의 ip를 mapping하는 함수
	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);  // 기본값은 somaxconn으로 알아서 세팅하도록 함
	static void Close(SOCKET socket);
};

template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, 
		reinterpret_cast<char*>(&optVal), sizeof(T));
}
