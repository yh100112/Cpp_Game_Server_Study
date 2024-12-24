#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"

void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	// echo
	cout << "OnRecv Len = " << len << endl;

	// ������ ������ ũ�⺸�� �ξ� ū 4096���� ��� ����
	// -> ���߿��� ���� �������� ���̸� �̸� ������ �� ���� ��Ȳ�� ����
	// �� sendbuffer�� �Ź� �� �� ����� �͵� �ȴ�!!
	// -> sendBuffer Pooling ������� �ذ� ����~ ( SendBufferChunk )
	//SendBufferRef sendBuffer = MakeShared<SendBuffer>(4096);
	//sendBuffer->CopyData(buffer, len); // ó�� �� ���� ��������

	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	::memcpy(sendBuffer->Buffer(), buffer, len);
	sendBuffer->Close(len);

	for (int32 i = 0; i < 5; i++)
		GSessionManager.Broadcast(sendBuffer);

	return len;
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend Len = " << len << endl;
}
