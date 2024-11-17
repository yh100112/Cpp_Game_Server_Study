#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ServerPacketHandler.h"

void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

// 완전체로 조립된 패킷이 왔다고 확신할 수 있는 상태
void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	ServerPacketHandler::HandlePacket(buffer, len);

	// 보내는 데이터 크기보다 훨씬 큰 4096으로 잡는 이유
	// -> 나중에는 보낼 데이터의 길이를 미리 예측할 수 없는 상황이 생김
	// 이 sendbuffer를 매번 한 번 만드는 것도 싫다!!
	// -> sendBuffer Pooling 기법으로 해결 가능~ ( SendBufferChunk )
	//SendBufferRef sendBuffer = MakeShared<SendBuffer>(4096);
	//sendBuffer->CopyData(buffer, len); // 처음 한 번은 복사해줌
}

void GameSession::OnSend(int32 len)
{
	cout << "OnSend Len = " << len << endl;
}
