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

		// iocpHandle(Completion Port)을 받아서 완료된 큐가 있는지 계속해서 체크하는 부분
		// 계속 체크한다고 부하가 있고 그런건 아님
		// 대기 시간을 설정할 수 있는데 INFINITE로 두면 워커 스레드가 일이 없으면 대기하고 있다가 일이 생기면 스레드 1개를 운영체제가 깨워서 일을 시키게 된다.
		// 2번째 인자 : 송수신된 바이트 개수가 얼만지 반환해주기 위한 변수
		// 세번째 인자 : accept 부분에서 세팅한 CP key 값을 다시 받아옴
		// 4번째 인자 : accept 부분에서 세팅한 overlapped pointer를 다시 받아옴
		BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, (ULONG_PTR*)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);

		if (ret == FALSE || bytesTransferred == 0)
		{
			// TODO : 연결 끊김
			continue;
		}

		// read가 아니면 crash 나도록 함
		ASSERT_CRASH(overlappedEx->type == IO_TYPE::READ);

		cout << "Recv Data IOCP = " << bytesTransferred << endl; // 얼마의 바이트를 받았는지 출력

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;

		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL); // 반복해서 처리해주기 위해 끝날 때 recv()를 걸어줘야 함
	}
}

/*
Overlapped 모델
- 비동기 입출력 함수가 완료되면, 쓰레드마다 apc큐에 일감이 쌓인다
- alertable wait 상태로 들어가게 되면 apc 큐를 비워준다 ( 콜백 함수를 호출한다 )
단점
- apc 큐가 쓰레드마다 있다는 것이 아쉬운 부분이다. -> 멀티스레드 환경에서 적절히 분배하는게 애매하다.
- alertable wait 자체도 부담이 좀 된다.
- overlapped event 방식은 소켓이랑 이벤트를 1대 1로 대응시키는 것 부터가 피곤한 일이고, 감시하는 것 자체가 64개 밖에 지원을 안하는 것도 문제이다.
*/

// IOCP (Completion Port) 모델
// - APC -> Completion Port (쓰레드마다 있는건 아니고 1개이다. 중앙에서 관리하는 APC 큐 같은 느낌. 다수의 스레드가 하나의 CP에서 일감을 받는다)
// - Alertable Wait -> Completion Port에 결과 처리를 하기 위해서는 GetQueuedCompletionStatus를 호출해준다.
// CreateIoCompletionPort
// GetQueuedCompletionStatus
// CP 자체가 스레드마다 배치하는것이 아니기 때문에 쓰레드랑 궁합이 굉장히 좋다
int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // 윈속 초기화
		return 0;

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0); // listen socket 생성
	if (listenSocket == INVALID_SOCKET)
		return 0;

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // bind in any ip
	serverAddr.sin_port = ::htons(7777);  // server port 7777

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) // 바인딩
		return 0;

	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;

	cout << "Accept" << endl;


	HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); // CP 생성하는 용도로 호출 ( 이때는 무조건 세팅값은 이렇게 해야 함 )

	for (int32 i = 0; i < 5; i++)
		GThreadManager->Launch([=]() { WorkerThreadMain(iocpHandle); }); // WorkerThreads

	// Main Thread = Accept 담당
	// 우선 iocp 구조가 어떻게 되는지 테스트 코드이므로 accept는 여기서 담당하도록 함 ( 나중에는 accept도 CP에서 처리하도록 만들 거임 )
	// 일단 테스트이니 논블로킹 소켓으로 할 필요도 없으므로 블로킹 소켓으로 세팅
	vector<Session*> sessionManager;
	while (true)
	{
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);

		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
			return 0;

		Session* session = xnew<Session>(); // xnew : 이걸로 세팅하면 #stomp allocator를 키면 아래에서 session을 날리면 crash 나도록 할 수 있음
		session->socket = clientSocket;
		sessionManager.push_back(session);

		cout << "Client Connected !" << endl;

		// 생성된 소켓을 CP에 등록 ( 이 소켓은 관찰 대상이다라는 것을 알려주는 것 ) - 소켓을 등록해서 관찰해달라 세팅하는 것도 CreateIoCompletionPort를 사용
		// 세번째 인자인 key값은 아무거나 넣어주면 됨. 우린 그냥 session값을 넣어줌
		::CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)session, 0);


		/*
		처음 연결을 받으면 최초 한 번 여기서 recv를 한 번 걸어준다. ( 처음으로 낚싯대를 던져주는 것 )
		당장 완료가 되거나 나중에 완료가 되는 등 완료된 시점에 어떤 스레드를 깨워서 일을 처리할 텐데
		일을 다 처리해준 다음에 이어가지고 또 recv를 하고 싶다면 일을 처리한 worker thread에서 wsarecv를 또 호출해줘야 한다.
		-> 콜백 함수(worker Thread)에서 이 recv()를 다시 호출해준다.
		*/
		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;
		OverlappedEx* overlappedEx = new OverlappedEx(); // 1대1 대응이 아니라 여러개의 클라가 들어올 수 있으므로 스택이 아닌 동적 메모리에 할당하여 넘겨줘야 함
		overlappedEx->type = IO_TYPE::READ;
		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL); 

		/*
		문제가 되는 상황
		유저가 게임 접속 종료 -> 이러면 CP에 등록한 session은 유효하지 않아서 크래시 발생

		문제를 방지하기 위한 방법
		WSARecv()를 걸어준 후 그 session은 절대로 날리지 못하도록 막아줘야 한다
		방법1. reference counting을 걸어줌
		
		Session* s = sessionManager.back();
		sessionManager.pop_back();
		xdelete(s);
		
		*/

		//::closesocket(session.socket);
		//::WSACloseEvent(wsaEvent);
	}

	GThreadManager->Join();

	// 윈속 종료
	::WSACleanup();
}