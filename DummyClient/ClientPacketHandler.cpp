#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"

void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case S_TEST:
		Handle_S_TEST(buffer, len);
		break;
	}
}

#pragma pack(1) // 1byte 단위로 관리하겠다
// [PKT_S_TEST][(가변데이터는 이 뒤에 넣어줌) BuffsListItem BuffsListItem BuffsListItem ]
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;
	};

	uint16 packetSize; // 공용 헤더
	uint16 packetId;  // 공용 헤더
	uint64 id; // 8byte
	uint32 hp; // 4byte
	uint16 attack; // 2byte

	// 가변 데이터 - 어찌 처리할까
	// 1) 문자열 (ex. name)
	// 2) 그냥 바이트 배열 (ex. 길드 이미지)
	// 3) 일반 리스트 ( vector )
	uint16 buffsOffset; // 가변데이트의 오프셋 ( 시작 위치 )
	uint16 buffsCount; // 가변데이터의 개수
	//vector<BuffsListItem> buffs;

	bool Validate()
	{
		uint32 size = 0;
		size += sizeof(PKT_S_TEST);
		size += buffsCount * sizeof(BuffsListItem);
		if (size != packetSize)
			return false;

		if (buffsOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;
	}
};
#pragma pack()

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	if (len < sizeof(PKT_S_TEST))
		return;

	PKT_S_TEST pkt;
	br >> pkt;

	if (pkt.Validate() == false)
		return;

	//cout << "ID : " << id << " HP : " << hp << " ATT : " << attack << endl;

	vector<PKT_S_TEST::BuffsListItem> buffs;

	buffs.resize(pkt.buffsCount);
	for (int32 i = 0; i < pkt.buffsCount; i++)
		br >> buffs[i];

	cout << "BufCount : " << pkt.buffsCount << endl;
	for (int32 i = 0; i < pkt.buffsCount; i++)
	{
		cout << "BuffInfo : " << buffs[i].buffId << " " << buffs[i].remainTime << endl;
	}
}
