#include "pch.h"
#include <iostream>

#include <winsock2.h> // windows socket 프로그래밍에 필요
#include <mswsock.h>  // 소켓 프로그래밍에 필요
#include <ws2tcpip.h> // 소켓 프로그래밍에 필요
#pragma comment(lib, "ws2_32.lib") // 속성에서 설정하기 귀찮으니 사용 라이브러리를 코드에 추가


int main()
{
	// 윈도우 소켓 초기화 (ws2_32 라이브러리 초기화)
	// 관련 정보가 wsaData에 채워짐
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	// ad : Address Family (AF_INET = IPv4, AF_INET6 = IPv6)
	// type : TCP(SOCK_STREAM) vs UDP(SOCK_DGRAM)
	// protocol : 0
	// return : descriptor
	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}

	// 연결할 목적지는? (IP주소 + Port) -> XX 아파트 YY 호
	SOCKADDR_IN serverAddr; // IPv4
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1"); // 구식 방식 ( deprecated )
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // 최신 방식

	serverAddr.sin_port = ::htons(7777); // 80 : HTTP
	// host to network short -> 호스트에서 네트워크 방식으로 바꿔주는 함수
	// Little-Endian vs Big-Endian
	// ex) 0x12345678 4바이트 정수
	// low [0x78][0x56][0x34][0x12] high : little 방식
	// low [0x12][0x34][0x56][0x78] high : big 방식    = network 표준

	if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Connect ErrorCode : " << errCode << endl;
		return 0;
	}

	// ---------------------------
	// 연결 성공! 이제부터 데이터 송수신 가능!

	cout << "Connected To Server!" << endl;

	while (true)
	{
		// TODO
		char sendBuffer[100] = "Hello World!";

		for (int32 i = 0; i < 10; i++)
		{
			int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
			if (resultCode == SOCKET_ERROR)
			{
				int32 errCode = ::WSAGetLastError();
				cout << "Send ErrorCode : " << errCode << endl;
				return 0;
			}
		}

		cout << "Send Data! Len = " << sizeof(sendBuffer) << endl;

		//char recvBuffer[100];

		//int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		//if (recvLen <= 0)
		//{
		//	int32 errCode = ::WSAGetLastError();
		//	cout << "Recv ErrorCode : " << errCode << endl;
		//	return 0;
		//}

		//cout << "Recv Data! Data = " << recvBuffer << endl;
		//cout << "Recv Data! Len = " << recvLen << endl;

		this_thread::sleep_for(1s);
	}

	// ---------------------------

	// 소켓 리소스 반환
	::closesocket(clientSocket);

	// 윈도우 소켓 종료
	::WSACleanup();
}