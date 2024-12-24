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

// Ŭ�� ������ �����ϸ� session ����ü�� ������ ����
struct Session
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[BUFSIZE] = {};
	int32 recvBytes = 0;
	int32 sendBytes = 0;
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
	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET) // �� ���ŷ ó��
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

	/*
	WSAEventSelect
	���ϰ� ���õ� ��Ʈ��ũ �̺�Ʈ�� [�̺�Ʈ ��ü]�� ���� ����
	select() �κ��� ������ ���� ����ε� WSAEventSelect������ �񵿱� ������� �ٲ�
	
	<�̺�Ʈ ��ü ���� �Լ���>
	���� : WSACreateEvent(�������� ��������� �� Manual-Reset + Non-Signaled ���� ����)
	���� : WSACloseEvent
	��ȣ ���� ���� : WSAWaitForMultipleEvents
	��ü���� ��Ʈ��ũ �̺�Ʈ �˾Ƴ��� : WSAEnumNetworkEvents

	< ���� <-> �̺�Ʈ ��ü ���� >
	���� ���� ��ŭ �̺�Ʈ ��ü�� �������� ��

	WSAEventSelect(socket, event, networkEvents);
	- ������ ��Ʈ��ũ �̺�Ʈ
		FD_ACCEPT	: ������ Ŭ�� ������ accept
		FD_READ		: ������ ���� ���� recv, recvfrom 
		FD_WRITE	: ������ �۽� ���� send, sendto
		FD_CLOSE	: ��밡 ���� ����
		FD_CONNECT	: ����� ���� ���� ���� �Ϸ�
		FD_OOB
	- ���ǻ���
		- WSAEventSelect �Լ��� ȣ���ϸ�, �ش� ������ �ڵ����� �ͺ��ŷ ���� ��ȯ
		- accept() �Լ��� �����ϴ� ������ listenSocket�� ������ �Ӽ��� ���´�
			- ���� clientSocket�� FD_READ, FD_WRITE ���� �ٽ� ��� �ʿ�
			- �幰�� WSAWOULDBLOCK ������ �� �� ������ ���� ó�� �ʿ�
	- �߿�
		- �̺�Ʈ �߻� ��. ������ ���� �Լ� ȣ���ؾ� ��
		- �ƴϸ� ���� ������ ���� ��Ʈ��ũ �̺�Ʈ�� �߻� X
		ex) FD_READ �̺�Ʈ ������ recv() ȣ���ؾ� �ϰ�, ���ϸ� FD_READ �ι� �ٽ� X
	
	WSAWaitForMultipleEvents
	- WSAEventSelect�� ���ؼ� �̺�Ʈ�� ��ϸ� �ϰ�, ���������� ������ �޴� �� ���⼭ ��
	���� 1) count. event
	���� 2) waitAll : ��� ��ٸ�? �ϳ��� �Ϸ� �Ǿ OK?
	���� 3) timeout : Ÿ�Ӿƿ�
	���� 4) ������ false
	return : �Ϸ�� ù��° �ε���

	WSAEnumNetworkEvents
	- ���������� � �̺�Ʈ�� ��ȯ�Ǿ������� �˷���
	���� 1) socket
	���� 2) eventObject : socket�� ������ �̺�Ʈ ��ü �ڵ��� �Ѱ��ָ�, �̺�Ʈ ��ü�� non-signaled
	���� 3) networkEvent : ��Ʈ��ũ �̺�Ʈ / ���� ������ ����
	*/

	vector<WSAEVENT> wsaEvents;
	vector<Session> sessions;
	sessions.reserve(100);

	WSAEVENT listenEvent = ::WSACreateEvent();
	wsaEvents.push_back(listenEvent);
	sessions.push_back(Session{ listenSocket }); // �̺�Ʈ�� ������ 1��1�� ���������ֱ� ����
	if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR) // �����ϰڴٰ� ����
		return 0;

	fd_set reads;
	fd_set writes;
	
	while (true)
	{
		int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED)
			continue;

		index -= WSA_WAIT_EVENT_0;

		//::WSAResetEvent(wsaEvents[index]); -> �� ����� WSAEnumNetworkEvents�� ���ԵǾ ���ص� ��
		WSANETWORKEVENTS networkEvents;
		if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
			continue;

		// Listener socket check
		if (networkEvents.lNetworkEvents & FD_ACCEPT)
		{
			// error check
			if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				continue;

			SOCKADDR_IN clientAddr;
			int32 addrLen = sizeof(clientAddr);

			
			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
			{
				cout << "Client connected" << endl;
				
				WSAEVENT clientEvent = ::WSACreateEvent();
				wsaEvents.push_back(clientEvent);
				sessions.push_back(Session{ clientSocket }); // �̺�Ʈ�� ������ 1��1�� ���������ֱ� ����
				if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
					return 0;

			}
		}

		// client session ���� üũ
		if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents * FD_WRITE)
		{
			// error check
			if ((networkEvents.lNetworkEvents & FD_READ) && (networkEvents.iErrorCode[FD_READ_BIT] != 0))
				continue;
			// error check
			if ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_WRITE_BIT] != 0))
				continue;

			Session& s = sessions[index];

			// read
			if (s.recvBytes == 0)
			{
				int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
				if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
				{
					// todo : remove session
					continue;
				}

				s.recvBytes = recvLen;
				cout << "recv data : " << recvLen << endl;
			}
			
			// write
			if (s.recvBytes > s.sendBytes)
			{
				int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
				if (sendLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
				{
					// todo: remove session
					continue;
				}

				s.sendBytes += sendLen;
				if (s.recvBytes == s.sendBytes)
				{
					s.recvBytes = 0;
					s.sendBytes = 0;
				}

				cout << "send data : " << sendLen << endl;

			}
		}

		// fd_close ó��
		if (networkEvents.lNetworkEvents & FD_CLOSE)
		{
			// todo : remove socket

		}
	}


	// ���� ����
	::WSACleanup();
}
