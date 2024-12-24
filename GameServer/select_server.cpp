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

	// Select �� = (select �Լ��� �ٽ��� �Ǵ�)
	// ���� �Լ� ȣ���� ������ ������ �̸� �� �� �ִ�!
	// ���� ��Ȳ)
	// ���Ź��ۿ� �����Ͱ� ���µ�, read �Ѵٰų�!
	// �۽Ź��۰� �� á�µ�, write �Ѵٰų�!
	// - ���ŷ ���� : ������ �������� �ʾƼ� ���ŷ�Ǵ� ��Ȳ ����
	// - ����ŷ ���� : ������ �������� �ʾƼ� ���ʿ��ϰ� �ݺ� üũ�ϴ� ��Ȳ�� ����

	// socket set
	// 1) �б�[ ] ����[ ] ����(OOB)[ ] ���� ��� ���
	// OutOfBand�� send() ������ ���� MSG_OOB�� ������ Ư���� ������. �޴� �ʿ����� recv OOB ������ �ؾ� ���� �� ����
	// 2) select(readSet, writeSet, exceptSet); -> ���� ���� ( ������ ���ϵ��� ���� )
	// 3) ��� �ϳ��� ������ �غ�Ǹ� ���� -> �����ڴ� �˾Ƽ� ���ŵ�
	// 4) ���� ���� üũ�ؼ� ����

	// ����� ��� ����
	// fd_set set; 
	// FD_ZERO	: ����				ex) FD_ZERO(set);
	// FD_SET	: ���� s�� �ִ´�		ex) FD_SET(s, &set);
	// FD_CLR	: ���� s�� ����		ex) FD_CLR(s, &set);
	// FD_ISSET : ���� s�� set�� ��������� 0�� �ƴ� ���� �����Ѵ�


	vector<Session> sessions;
	sessions.reserve(100);

	fd_set reads;
	fd_set writes;

	while (true)
	{
		// ���� �� �ʱ�ȭ
		// �Ź� ������ ���鼭 �Ź� �ʱ�ȭ�� -> �Ź� �ʱ�ȭ�ϰ� �ٽ� �Ʒ����� ���� ���Ͽ� �����
		// select���� �Ź� �����ڴ� ���Ű� �ǹǷ� reads, writes�� ó�� ���¶� �ٸ��Ƿ� �Ź� �ʱ�ȭ ����� ��!
		// select�� ȣ��� ������ reads, writes�� �ִ� �����Ͱ� ���� ���� �� �ִ�.
		FD_ZERO(&reads);
		FD_ZERO(&writes);

		// ListenSocket ���
		// listen�� ��� �ִ� �Ű� accept�� �� ����� �ִ��� ��� �ִ� �Ŷ� read�� �־���� ��
		FD_SET(listenSocket, &reads);

		// ���� ���
		for (Session& s : sessions)
		{
			if (s.recvBytes <= s.sendBytes)
				FD_SET(s.socket, &reads); 
			else
				FD_SET(s.socket, &writes);
		}

		// [�ɼ�] ������ timeout ���� ���� ����
		// �ϳ��� �غ� �� �ְ� ������ return���ְ� �ƴϸ� ���⼭ ���� ��� ( ������ ������ �ð��� null�� ������ )
		// -> ��ȯ�� �� �������� ���� �ֵ��� �����ڷ� �����ؼ� ���� ���� ����ϸ� �غ� �� �ֵ鸸 ���� �ȴ�.
		int32 retVal = ::select(0, &reads, &writes, nullptr, nullptr);
		if (retVal == SOCKET_ERROR)
			break;

		// Listener ���� üũ
		if (FD_ISSET(listenSocket, &reads)) // select�� ����Ͽ� accept�� �غ� �� �ֵ鸸 set�� �����ִ� ���� -> �غ�� �ֵ��� üũ!
		{
			SOCKADDR_IN clientAddr;
			int32 addrLen = sizeof(clientAddr);
			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
			{
				cout << "Client Connected" << endl;
				sessions.push_back(Session{ clientSocket }); // ����� Ŭ���̾�Ʈ�� session�� �߰�
			}
		}

		// ������ ���� üũ
		for (Session& s : sessions)
		{
			// Read
			if (FD_ISSET(s.socket, &reads))
			{
				int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
				if (recvLen <= 0)
				{
					// TODO : sessions���� ����
					continue;
				}

				s.recvBytes = recvLen;
			}

			// Write
			if (FD_ISSET(s.socket, &writes))
			{
				// ���ŷ ��� -> ��� ������ �� ����
				// ����ŷ ��� -> �Ϻθ� ���� ���� ���� (���� ���� ���� ��Ȳ�� ����)
				int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
				if (sendLen == SOCKET_ERROR)
				{
					// TODO : sessions���� ����
					continue;
				}

				s.sendBytes += sendLen;
				if (s.recvBytes == s.sendBytes)
				{
					s.recvBytes = 0;
					s.sendBytes = 0;
				}
			}
		}
	}


	// ���� ����
	::WSACleanup();
}