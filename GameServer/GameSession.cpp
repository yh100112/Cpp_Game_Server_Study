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

// ����ü�� ������ ��Ŷ�� �Դٰ� Ȯ���� �� �ִ� ����
int32 GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketHeader header = *((PacketHeader*)buffer);
	cout << "Packet Id : " << header.id << "size : " << header.size << endl;

	// ������ ������ ũ�⺸�� �ξ� ū 4096���� ��� ����
	// -> ���߿��� ���� �������� ���̸� �̸� ������ �� ���� ��Ȳ�� ����
	// �� sendbuffer�� �Ź� �� �� ����� �͵� �ȴ�!!
	// -> sendBuffer Pooling ������� �ذ� ����~ ( SendBufferChunk )
	//SendBufferRef sendBuffer = MakeShared<SendBuffer>(4096);
	//sendBuffer->CopyData(buffer, len); // ó�� �� ���� ��������

	return len;
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend Len = " << len << endl;
}
