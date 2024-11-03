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

	/*
	WSAEventSelect
	소켓과 관련된 네트워크 이벤트를 [이벤트 객체]를 통해 감지
	select() 부분이 원래는 동기 방식인데 WSAEventSelect에서는 비동기 방식으로 바뀜
	
	<이벤트 객체 관련 함수들>
	생성 : WSACreateEvent(수동으로 리셋해줘야 함 Manual-Reset + Non-Signaled 상태 시작)
	삭제 : WSACloseEvent
	신호 상태 감지 : WSAWaitForMultipleEvents
	구체적인 네트워크 이벤트 알아내기 : WSAEnumNetworkEvents

	< 소켓 <-> 이벤트 객체 연동 >
	소켓 개수 만큼 이벤트 객체를 만들어줘야 함

	WSAEventSelect(socket, event, networkEvents);
	- 관찰할 네트워크 이벤트
		FD_ACCEPT	: 접속한 클라가 있으면 accept
		FD_READ		: 데이터 수신 가능 recv, recvfrom 
		FD_WRITE	: 데이터 송신 가능 send, sendto
		FD_CLOSE	: 상대가 접속 종료
		FD_CONNECT	: 통신을 위한 연결 절차 완료
		FD_OOB
	- 주의사항
		- WSAEventSelect 함수를 호출하면, 해당 소켓은 자동으로 넌블로킹 모드로 전환
		- accept() 함수가 리턴하는 소켓은 listenSocket과 동일한 속성을 갖는다
			- 따라서 clientSocket은 FD_READ, FD_WRITE 등을 다시 등록 필요
			- 드물게 WSAWOULDBLOCK 오류가 뜰 수 있으니 예외 처리 필요
	- 중요
		- 이벤트 발생 시. 적절한 소켓 함수 호출해야 함
		- 아니면 다음 번에는 동일 네트워크 이벤트가 발생 X
		ex) FD_READ 이벤트 떴으면 recv() 호출해야 하고, 안하면 FD_READ 두번 다시 X
	
	WSAWaitForMultipleEvents
	- WSAEventSelect를 통해서 이벤트를 등록만 하고, 실질적으로 통지를 받는 건 여기서 함
	인자 1) count. event
	인자 2) waitAll : 모두 기다림? 하나만 완료 되어도 OK?
	인자 3) timeout : 타임아웃
	인자 4) 지금은 false
	return : 완료된 첫번째 인덱스

	WSAEnumNetworkEvents
	- 최종적으로 어떤 이벤트가 반환되었는지를 알려줌
	인자 1) socket
	인자 2) eventObject : socket과 연동된 이벤트 객체 핸들을 넘겨주면, 이벤트 객체를 non-signaled
	인자 3) networkEvent : 네트워크 이벤트 / 오류 정보가 저장
	*/

	vector<WSAEVENT> wsaEvents;
	vector<Session> sessions;
	sessions.reserve(100);

	WSAEVENT listenEvent = ::WSACreateEvent();
	wsaEvents.push_back(listenEvent);
	sessions.push_back(Session{ listenSocket }); // 이벤트와 세션을 1대1로 대응시켜주기 위함
	if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR) // 관찰하겠다고 선언
		return 0;

	fd_set reads;
	fd_set writes;
	
	while (true)
	{
		int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED)
			continue;

		index -= WSA_WAIT_EVENT_0;

		//::WSAResetEvent(wsaEvents[index]); -> 이 기능이 WSAEnumNetworkEvents에 포함되어서 안해도 됨
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
				sessions.push_back(Session{ clientSocket }); // 이벤트와 세션을 1대1로 대응시켜주기 위함
				if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
					return 0;

			}
		}

		// client session 소켓 체크
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

		// fd_close 처리
		if (networkEvents.lNetworkEvents & FD_CLOSE)
		{
			// todo : remove socket

		}
	}


	// 윈속 종료
	::WSACleanup();
}
