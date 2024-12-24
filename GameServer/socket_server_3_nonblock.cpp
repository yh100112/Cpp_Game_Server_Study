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

int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	// ���ŷ(Blocking) ����
	// accept			-> ������ Ŭ�� ���� ��
	// connect			-> ���� ���� �������� ��
	// send, sendto		-> ��û�� �����͸� �۽� ���ۿ� �������� ��
	// recv, recvfrom -> ���� ���ۿ� ������ �����Ͱ� �ְ�, �̸� �������� ���ۿ� �������� ��

	// ����ŷ(Non-Blocking)

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		return 0;

	u_long on = 1;
	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET) // ����ŷ ���
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

	SOCKADDR_IN clientAddr;
	int32 addrLen = sizeof(clientAddr);

	// Accept 
	while (true)
	{
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		// ����ŷ���� �϶�� �����Ƿ� accept���� ��ٸ��� �ʰ� �ٷ� �������ͼ� ���⿡ ���� �� �ִ�.
		if (clientSocket == INVALID_SOCKET) 
		{
			// ���� accept()���� ����߾�� �ߴµ�... �ʰ� ����ŷ���� �϶���ؼ� �����ݾ�!
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue; // (����ŷ�̹Ƿ� accept�� �� ������ ���� ���� ��)

			// Error
			break;
		}

		cout << "Client Connected!" << endl;

		// Recv
		while (true)
		{
			char recvBuffer[1000];
			int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
			// ����ŷ���� �ٲ������ recv���� �ٷ� �������ͼ� �� ���� �ڵ带 �� ����� ���� �� ����
			if (recvLen == SOCKET_ERROR)
			{
				// ���� recv()���� ����߾�� �ߴµ�... �ʰ� ����ŷ���� �϶���ؼ� �����ݾ�!
				if (::WSAGetLastError() == WSAEWOULDBLOCK)
					continue;

				// Error
				break;
			}
			else if (recvLen == 0)
			{
				// ���� ����
				break;
			}

			cout << "Recv Data Len = " << recvLen << endl;

			// Send
			while (true)
			{
				if (::send(clientSocket, recvBuffer, recvLen, 0) == SOCKET_ERROR)
				{
					// ���� send()���� ����߾�� �ߴµ�... �ʰ� ����ŷ���� �϶���ؼ� �����ݾ�!
					if (::WSAGetLastError() == WSAEWOULDBLOCK)
						continue;
					// Error
					break;
				}

				cout << "Send Data ! Len = " << recvLen << endl;
				break;
			}
		}
	}


	// ���� ����
	::WSACleanup();
}