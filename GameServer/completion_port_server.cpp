#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Memory.h"

enum IO_TYPE
{
	READ,
	WRITE,
	ACCEPT,
	CONNECT,
};

void HandleError(const char* cause)
{
	int32 errCode = ::WSAGetLastError();
	cout << cause << " ErrorCode : " << errCode << endl;
}

const int32 BUFSIZE = 1000;

struct Session
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[BUFSIZE] = {};
	int32 recvBytes = 0;
};
struct OverlappedEx
{
	WSAOVERLAPPED overlapped = {};
	int32 type = 0; // read, write, accept, connect ...
};

void WorkerThreadMain(HANDLE iocpHandle)
{
	while (true)
	{
		DWORD bytesTransferred = 0;
		Session* session = nullptr;
		OverlappedEx* overlappedEx = nullptr;

		// iocpHandle(Completion Port)�� �޾Ƽ� �Ϸ�� ť�� �ִ��� ����ؼ� üũ�ϴ� �κ�
		// ��� üũ�Ѵٰ� ���ϰ� �ְ� �׷��� �ƴ�
		// ��� �ð��� ������ �� �ִµ� INFINITE�� �θ� ��Ŀ �����尡 ���� ������ ����ϰ� �ִٰ� ���� ����� ������ 1���� �ü���� ������ ���� ��Ű�� �ȴ�.
		// 2��° ���� : �ۼ��ŵ� ����Ʈ ������ ���� ��ȯ���ֱ� ���� ����
		// ����° ���� : accept �κп��� ������ CP key ���� �ٽ� �޾ƿ�
		// 4��° ���� : accept �κп��� ������ overlapped pointer�� �ٽ� �޾ƿ�
		BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, (ULONG_PTR*)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);

		if (ret == FALSE || bytesTransferred == 0)
		{
			// TODO : ���� ����
			continue;
		}

		// read�� �ƴϸ� crash ������ ��
		ASSERT_CRASH(overlappedEx->type == IO_TYPE::READ);

		cout << "Recv Data IOCP = " << bytesTransferred << endl; // ���� ����Ʈ�� �޾Ҵ��� ���

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;

		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL); // �ݺ��ؼ� ó�����ֱ� ���� ���� �� recv()�� �ɾ���� ��
	}
}

/*
Overlapped ��
- �񵿱� ����� �Լ��� �Ϸ�Ǹ�, �����帶�� apcť�� �ϰ��� ���δ�
- alertable wait ���·� ���� �Ǹ� apc ť�� ����ش� ( �ݹ� �Լ��� ȣ���Ѵ� )
����
- apc ť�� �����帶�� �ִٴ� ���� �ƽ��� �κ��̴�. -> ��Ƽ������ ȯ�濡�� ������ �й��ϴ°� �ָ��ϴ�.
- alertable wait ��ü�� �δ��� �� �ȴ�.
- overlapped event ����� �����̶� �̺�Ʈ�� 1�� 1�� ������Ű�� �� ���Ͱ� �ǰ��� ���̰�, �����ϴ� �� ��ü�� 64�� �ۿ� ������ ���ϴ� �͵� �����̴�.
*/

// IOCP (Completion Port) ��
// - APC -> Completion Port (�����帶�� �ִ°� �ƴϰ� 1���̴�. �߾ӿ��� �����ϴ� APC ť ���� ����. �ټ��� �����尡 �ϳ��� CP���� �ϰ��� �޴´�)
// - Alertable Wait -> Completion Port�� ��� ó���� �ϱ� ���ؼ��� GetQueuedCompletionStatus�� ȣ�����ش�.
// CreateIoCompletionPort
// GetQueuedCompletionStatus
// CP ��ü�� �����帶�� ��ġ�ϴ°��� �ƴϱ� ������ ������� ������ ������ ����
int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // ���� �ʱ�ȭ
		return 0;

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0); // listen socket ����
	if (listenSocket == INVALID_SOCKET)
		return 0;

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // bind in any ip
	serverAddr.sin_port = ::htons(7777);  // server port 7777

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) // ���ε�
		return 0;

	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;

	cout << "Accept" << endl;


	HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); // CP �����ϴ� �뵵�� ȣ�� ( �̶��� ������ ���ð��� �̷��� �ؾ� �� )

	for (int32 i = 0; i < 5; i++)
		GThreadManager->Launch([=]() { WorkerThreadMain(iocpHandle); }); // WorkerThreads

	// Main Thread = Accept ���
	// �켱 iocp ������ ��� �Ǵ��� �׽�Ʈ �ڵ��̹Ƿ� accept�� ���⼭ ����ϵ��� �� ( ���߿��� accept�� CP���� ó���ϵ��� ���� ���� )
	// �ϴ� �׽�Ʈ�̴� ����ŷ �������� �� �ʿ䵵 �����Ƿ� ���ŷ �������� ����
	vector<Session*> sessionManager;
	while (true)
	{
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);

		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
			return 0;

		Session* session = xnew<Session>(); // xnew : �̰ɷ� �����ϸ� #stomp allocator�� Ű�� �Ʒ����� session�� ������ crash ������ �� �� ����
		session->socket = clientSocket;
		sessionManager.push_back(session);

		cout << "Client Connected !" << endl;

		// ������ ������ CP�� ��� ( �� ������ ���� ����̴ٶ�� ���� �˷��ִ� �� ) - ������ ����ؼ� �����ش޶� �����ϴ� �͵� CreateIoCompletionPort�� ���
		// ����° ������ key���� �ƹ��ų� �־��ָ� ��. �츰 �׳� session���� �־���
		::CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)session, 0);


		/*
		ó�� ������ ������ ���� �� �� ���⼭ recv�� �� �� �ɾ��ش�. ( ó������ ���˴븦 �����ִ� �� )
		���� �Ϸᰡ �ǰų� ���߿� �Ϸᰡ �Ǵ� �� �Ϸ�� ������ � �����带 ������ ���� ó���� �ٵ�
		���� �� ó������ ������ �̾���� �� recv�� �ϰ� �ʹٸ� ���� ó���� worker thread���� wsarecv�� �� ȣ������� �Ѵ�.
		-> �ݹ� �Լ�(worker Thread)���� �� recv()�� �ٽ� ȣ�����ش�.
		*/
		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;
		OverlappedEx* overlappedEx = new OverlappedEx(); // 1��1 ������ �ƴ϶� �������� Ŭ�� ���� �� �����Ƿ� ������ �ƴ� ���� �޸𸮿� �Ҵ��Ͽ� �Ѱ���� ��
		overlappedEx->type = IO_TYPE::READ;
		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL); 

		/*
		������ �Ǵ� ��Ȳ
		������ ���� ���� ���� -> �̷��� CP�� ����� session�� ��ȿ���� �ʾƼ� ũ���� �߻�

		������ �����ϱ� ���� ���
		WSARecv()�� �ɾ��� �� �� session�� ����� ������ ���ϵ��� ������� �Ѵ�
		���1. reference counting�� �ɾ���
		
		Session* s = sessionManager.back();
		sessionManager.pop_back();
		xdelete(s);
		
		*/

		//::closesocket(session.socket);
		//::WSACloseEvent(wsaEvent);
	}

	GThreadManager->Join();

	// ���� ����
	::WSACleanup();
}