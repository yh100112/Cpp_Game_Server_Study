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

#pragma pack(1) // 1byte ������ �����ϰڴ�
// [PKT_S_TEST][(���������ʹ� �� �ڿ� �־���) BuffsListItem BuffsListItem BuffsListItem ]
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;
	};

	uint16 packetSize; // ���� ���
	uint16 packetId;  // ���� ���
	uint64 id; // 8byte
	uint32 hp; // 4byte
	uint16 attack; // 2byte

	// ���� ������ - ���� ó���ұ�
	// 1) ���ڿ� (ex. name)
	// 2) �׳� ����Ʈ �迭 (ex. ��� �̹���)
	// 3) �Ϲ� ����Ʈ ( vector )
	uint16 buffsOffset; // ��������Ʈ�� ������ ( ���� ��ġ )
	uint16 buffsCount; // ������������ ����
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
