#include "pch.h"
#include <iostream>
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"	
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ServerPacketHandler.h"

// ��Ŷ ����ȭ (Serialization)

#pragma pack(1) // 1byte ������ �����ϰڴ�
struct PKT_S_TEST
{
public:
	uint64 id; // 8byte
	uint32 hp; // 4byte
	uint16 attack; // 2byte
};
#pragma pack()

int main()
{
	PKT_S_TEST pkt;
	pkt.hp = 1;
	pkt.hp = 2;
	pkt.attack = 3;

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
		vector<BuffData> buffs{ BuffData {100, 1.5f}, BuffData{200, 2.3f}, BuffData{300, 0.7f} };
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_TEST(1001, 100, 10, buffs);


		GSessionManager.Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}