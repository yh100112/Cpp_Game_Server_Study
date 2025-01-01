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

	SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		HandleError("Socket");
		return 0;
	}

	SOCKADDR_IN serverAddr; // IPv4
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);

	if (::bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		HandleError("Bind");
		return 0;
	}

	while (true)
	{
		SOCKADDR_IN clientAddr;
		::memset(&clientAddr, 0, sizeof(clientAddr));
		int32 addrLen = sizeof(clientAddr);

		char recvBuffer[1000];

		int32 recvLen = ::recvfrom(serverSocket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)&clientAddr, &addrLen);

		if (recvLen <= 0)
		{
			HandleError("RecvFrom");
			return 0;
		}

		cout << "Recv Data! Data = " << recvBuffer << endl;
		cout << "Recv Data! Len = " << recvLen << endl;

		int32 errorCode = ::sendto(serverSocket, recvBuffer, recvLen, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
		
		if (errorCode == SOCKET_ERROR)
		{
			HandleError("SendTo");
			return 0;
		}

		cout << "Recv Data! Len = " << recvLen << endl;
	}


	// �ɼ��� �ؼ��ϰ� ó���� ��ü
	// ���� �ڵ� -> SOL_SOCKET
	// IPv4 -> IPPROTO_IP
	// TCP �������� -> IPPROTO_TCP

	// SO_KEEPALIVE = �ֱ������� ���� ���� Ȯ�� ���� ( TCP Only )
	// ������ �Ҹ��ҹ����� ���� ���´ٸ�?
	// �ֱ������� TCP �������� ���� ���� Ȯ�� -> ������ ���� ����
	bool enable = true;
	::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));

	// SO_LINGER = �����ϴ�.
	// �۽� ���ۿ� �ִ� �����͸� ���� ���ΰ�? ���� ���ΰ�?
	// onoff = 0�̸� closesocket()�� �ٷ� ����. �ƴϸ� linger�ʸ�ŭ ��� (default 0)
	// linger : ��� �ð�
	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 5;
	::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

	// half-close
	// SD_SEND : send ���´�.
	// SD_RECEIVE : recv ���´�
	// SD_BOTH : �� �� ���´�
	::shutdown(serverSocket, SD_SEND); // ���� ���̻� ������ ���� ���� ����. ( closesocket() ���� ����� �� - ����ϰ� )
	// ���� ���ҽ� ��ȯ ( ���� �ݱ� )
	// send -> closesocket
	::closesocket(serverSocket);

	// �ۼ��� ���� ũ�⸦ �˰� ���� �� ���
	// SO_SNDBUF = �۽� ���� ũ��
	// SO_RCVBUF = ���� ���� ũ��
	int32 sendBufferSize;
	int32 optionLen = sizeof(sendBufferSize);
	::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &optionLen);
	cout << "�۽� ���� ũ�� : " << sendBufferSize << endl;
	int32 recvBufferSize;
	int32 optionLen = sizeof(recvBufferSize);
	::getsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufferSize, &optionLen);
	cout << "���� ���� ũ�� : " << recvBufferSize << endl;

	// SO_REUSEADDR
	// �̹� ��� ���� ip �ּ� �� port ���� ( ������ ���� �� port�� ���� ����ϰڴ�! )
	// ���� �ܰ迡�� ���϶�� �ϴ� �۾�
	{
		bool enable = true;
		::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
	}

	// IPPROTO_TCP
	// TCP_NODELAY = Nagle �˰��� �۵� ����
	// �����Ͱ� ����� ũ�� ������, �׷��� ������ �����Ͱ� ����� ���� ������ ���!
	// ���� : ���� ��Ŷ�� ���ʿ��ϰ� ���� �����Ǵ� ���� ����
	// ���� : ���� �ð� ����
	// ���ӿ����� �Ϲ������� �� �ɼ��� ��
	{
		bool enable = true;
		::setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
	}

	// ���� ����
	::WSACleanup();
}