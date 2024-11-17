#include "pch.h"
#include <iostream>
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"	
#include "GameSession.h"
#include "GameSessionManager.h"

int main()
{
	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>,
		100  // 동접 100개 예약해달라
	);

	ASSERT_CRASH(service->Start());

	// iocp에 들어온 애를 감지하는 worker thread 
	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();  // iocp core에 들어온 일감을 감지하고 있다면 처리해준다.
				}
			});
	}

	char sendData[] = "Hello World";

	while (true)
	{
		SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

		BYTE* buffer = sendBuffer->Buffer();
		((PacketHeader*)buffer)->size = (sizeof(sendData) + sizeof(PacketHeader));
		((PacketHeader*)buffer)->id = 1; // 1 : Hello Msg
		::memcpy(&buffer[4], sendData, sizeof(sendData)); // 0 ~ 3은 4byte로 이미 패킷헤더가 2byte, 2byte 먹고 있으므로 실제 보낼 데이터는 4번째 인덱스부터 넣음
		sendBuffer->Close((sizeof(sendData) + sizeof(PacketHeader)));

		GSessionManager.Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}