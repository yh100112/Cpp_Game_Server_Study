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

// 클라가 서버에 접속하면 session 구조체로 정보를 관리
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
	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET) // 논 블로킹 처리
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

	// Select 모델 = (select 함수가 핵심이 되는)
	// 소켓 함수 호출이 성공할 시점을 미리 알 수 있다!
	// 문제 상황)
	// 수신버퍼에 데이터가 없는데, read 한다거나!
	// 송신버퍼가 꽉 찼는데, write 한다거나!
	// - 블로킹 소켓 : 조건이 만족되지 않아서 블로킹되는 상황 예방
	// - 논블로킹 소켓 : 조건이 만족되지 않아서 불필요하게 반복 체크하는 상황을 예방

	// socket set
	// 1) 읽기[ ] 쓰기[ ] 예외(OOB)[ ] 관찰 대상 등록
	// OutOfBand는 send() 마지막 인자 MSG_OOB로 보내는 특별한 데이터. 받는 쪽에서도 recv OOB 세팅을 해야 읽을 수 있음
	// 2) select(readSet, writeSet, exceptSet); -> 관찰 시작 ( 관찰할 소켓들을 넣음 )
	// 3) 적어도 하나의 소켓이 준비되면 리턴 -> 낙오자는 알아서 제거됨
	// 4) 남은 소켓 체크해서 진행

	// 만드는 방법 예시
	// fd_set set; 
	// FD_ZERO	: 비운다				ex) FD_ZERO(set);
	// FD_SET	: 소켓 s를 넣는다		ex) FD_SET(s, &set);
	// FD_CLR	: 소켓 s를 제거		ex) FD_CLR(s, &set);
	// FD_ISSET : 소켓 s가 set에 들어있으면 0이 아닌 값을 리턴한다


	vector<Session> sessions;
	sessions.reserve(100);

	fd_set reads;
	fd_set writes;

	while (true)
	{
		// 소켓 셋 초기화
		// 매번 루프를 돌면서 매번 초기화함 -> 매번 초기화하고 다시 아래에서 새로 소켓에 등록함
		// select에서 매번 낙오자는 제거가 되므로 reads, writes가 처음 상태랑 다르므로 매번 초기화 해줘야 함!
		// select가 호출된 다음에 reads, writes에 있는 데이터가 날라가 있을 수 있다.
		FD_ZERO(&reads);
		FD_ZERO(&writes);

		// ListenSocket 등록
		// listen이 듣고 있는 거고 accept을 할 대상이 있는지 듣고 있는 거라서 read에 넣어줘야 함
		FD_SET(listenSocket, &reads);

		// 소켓 등록
		for (Session& s : sessions)
		{
			if (s.recvBytes <= s.sendBytes)
				FD_SET(s.socket, &reads); 
			else
				FD_SET(s.socket, &writes);
		}

		// [옵션] 마지막 timeout 인자 설정 가능
		// 하나라도 준비가 된 애가 있으면 return해주고 아니면 여기서 무한 대기 ( 마지막 인자인 시간을 null로 했으니 )
		// -> 반환될 때 적합하지 않은 애들은 낙오자로 제거해서 실제 여길 통과하면 준비가 된 애들만 남게 된다.
		int32 retVal = ::select(0, &reads, &writes, nullptr, nullptr);
		if (retVal == SOCKET_ERROR)
			break;

		// Listener 소켓 체크
		if (FD_ISSET(listenSocket, &reads)) // select를 통과하여 accept될 준비가 된 애들만 set에 남아있는 상태 -> 준비된 애들을 체크!
		{
			SOCKADDR_IN clientAddr;
			int32 addrLen = sizeof(clientAddr);
			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
			{
				cout << "Client Connected" << endl;
				sessions.push_back(Session{ clientSocket }); // 연결된 클라이언트를 session에 추가
			}
		}

		// 나머지 소켓 체크
		for (Session& s : sessions)
		{
			// Read
			if (FD_ISSET(s.socket, &reads))
			{
				int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
				if (recvLen <= 0)
				{
					// TODO : sessions에서 제거
					continue;
				}

				s.recvBytes = recvLen;
			}

			// Write
			if (FD_ISSET(s.socket, &writes))
			{
				// 블로킹 모드 -> 모든 데이터 다 보냄
				// 논블로킹 모드 -> 일부만 보낼 수가 있음 (상대방 수신 버퍼 상황에 따라)
				int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
				if (sendLen == SOCKET_ERROR)
				{
					// TODO : sessions에서 제거
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


	// 윈속 종료
	::WSACleanup();
}