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
	WSAOVERLAPPED overlapped = {};
};

int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		return 0;

	u_long on = 1;
	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET) // no blocking
		return 0;

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;

	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;

	cout << "Accept" << endl;

	// Overlapped IO (�񵿱� + ����ŷ)
	// - Overlapped �Լ��� �Ǵ� (WSARecv, WSASend)
	// - Overlapped �Լ��� �����ߴ��� Ȯ�� ��
	// -> ���� :  ��� �� ó��
	// -> ���� :  ������ Ȯ��

	// WSASend
	// WSARecv
	// 1) �񵿱� ����� ����
	// 2) WSABUF �迭�� ���� �ּ� + ���� // Scatter-Gather
	// 3) ������/���� ����Ʈ ��
	// 4) �� �ɼ��ε� 0
	// 5) WSAOVERLAPPED ����ü �ּҰ�
	// 6) ������� �Ϸ�Ǹ� OS�� ȣ���� �ݹ� �Լ�

	// Overlapped �� (�̺�Ʈ ���)
	// - �񵿱� ����� �����ϴ� ���� ���� + ���� �ޱ� ���� �̺�Ʈ ��ü ����
	// - �񵿱� ����� �Լ� ȣ�� (1���� ���� �̺�Ʈ ��ü�� ���� �Ѱ���)
	// - �񵿱� �۾��� �ٷ� �Ϸ���� ������, WSA_IO_PENDING ���� �ڵ�
	// �ü���� �̺�Ʈ ��ü�� signaled ���·� ���� �Ϸ� ���� �˷���
	// - WSAWaitForMultipleEvents �Լ� ȣ���ؼ� �̺�Ʈ ��ü�� signal �Ǻ�
	// - WSAGetOverlappedResult ȣ���ؼ� �񵿱� ����� ��� Ȯ�� �� ������ ó��

	// 1) �񵿱� ����
	// 2) �Ѱ��� overlapped ����ü
	// 3) ���۵� ����Ʈ ��
	// 4) �񵿱� ����� �۾��� ���������� �������?
	// false
	// 5) �񵿱� ����� �۾� ���� �ΰ� ����. ���� ��� �� ��.
	// WSAGetOverlappedResult

	while (true)
	{
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);

		SOCKET clientSocket;
		while (true)
		{
			clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
				break;

			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			// ���� �ִ� ��Ȳ
			return 0;
		}

		Session session = Session{ clientSocket };
		WSAEVENT wsaEvent = ::WSACreateEvent();
		session.overlapped.hEvent = wsaEvent;

		cout << "Client Connected !" << endl;

		while (true)
		{
			WSABUF wsaBuf;
			wsaBuf.buf = session.recvBuffer;
			wsaBuf.len = BUFSIZE;

			DWORD recvLen = 0;
			DWORD flags = 0;
			// recv�� �񵿱� ������� ����
			if (::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, nullptr) == SOCKET_ERROR)
			{
				if (::WSAGetLastError() == WSA_IO_PENDING) // ���� �����Ͱ� ��� pending ������ ���̰� ������ ��Ȳ�� �ƴ� ��Ȳ
				{
					// Pending ( ���߿� �Ϸ� ������ ���� �� üũ )
					::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
					::WSAGetOverlappedResult(session.socket, &session.overlapped, &recvLen, FALSE, &flags);
				}
				else
				{
					// TODO : ���� �ִ� ��Ȳ
					break;
				}
			}

			cout << "Data Recv Len = " << recvLen << endl;
		}

		::closesocket(session.socket);
		::WSACloseEvent(wsaEvent);
	}

	// ���� ����
	::WSACleanup();
}