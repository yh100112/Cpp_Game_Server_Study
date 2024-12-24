#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "SocketUtils.h"
#include "Listener.h"

int main()
{
	Listener listener;
	listener.StartAccept(NetAddress(L"127.0.0.1", 7777));

	// iocp�� ���� �ָ� �����ϴ� worker thread 
	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					GlocpCore.Dispatch();  // iocp core�� ���� �ϰ��� �����ϰ� �ִٸ� ó�����ش�.
				}
			});
	}

	GThreadManager->Join();
}