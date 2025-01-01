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
		100  // ���� 100�� �����ش޶�
	);

	ASSERT_CRASH(service->Start());

	// iocp�� ���� �ָ� �����ϴ� worker thread 
	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();  // iocp core�� ���� �ϰ��� �����ϰ� �ִٸ� ó�����ش�.
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
		::memcpy(&buffer[4], sendData, sizeof(sendData)); // 0 ~ 3�� 4byte�� �̹� ��Ŷ����� 2byte, 2byte �԰� �����Ƿ� ���� ���� �����ʹ� 4��° �ε������� ����
		sendBuffer->Close((sizeof(sendData) + sizeof(PacketHeader)));

		GSessionManager.Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}