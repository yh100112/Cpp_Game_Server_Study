#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "Service.h"
#include "Session.h"	

class GameSession : public Session
{

};

int main()
{
	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<Session>,
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

	GThreadManager->Join();
}