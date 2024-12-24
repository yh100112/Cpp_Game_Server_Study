#pragma once
#include "NetAddress.h"

/*------------------
	SocketUtils
------------------*/
// ȣ���ϴ� �Լ����� �̷��� �����ؼ� ����ϴ°� ����.
// �� �ʿ��� ���ڵ鸸 ����� �� �־��ֱ� ������ (�ȿ��� �����ϹǷ�) �������̳� ��뼺�� ���� �ȴ�.
class SocketUtils
{
public:
	static LPFN_CONNECTEX		ConnectEx; // connectex��� �Լ��� �����Ͱ� ��
	static LPFN_DISCONNECTEX	DisconnectEx;
	static LPFN_ACCEPTEX		AcceptEx;

public:
	static void Init();
	static void Clear();

	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET CreateSocket(); // ������ tcp ������ ����� �Լ� ( ���� ����� ���� ���� ����Ƿ� ���� )

	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
	static bool SetReuseAddress(SOCKET socket, bool flag);
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	static bool SetSendBufferSize(SOCKET socket, int32 size);
	static bool SetTcpNoDelay(SOCKET socket, bool flag);
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

	static bool Bind(SOCKET socket, NetAddress netAddr); // socket�� Ư�� ip�� mapping�ϴ� �Լ�
	static bool BindAnyAddress(SOCKET socket, uint16 port); // socket�� ������ ip�� mapping�ϴ� �Լ�
	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);  // �⺻���� somaxconn���� �˾Ƽ� �����ϵ��� ��
	static void Close(SOCKET socket);
};

template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, 
		reinterpret_cast<char*>(&optVal), sizeof(T));
}
