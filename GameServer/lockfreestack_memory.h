#pragma once

//// ������ �ϳ��� ����� ���� Node�� ����� �ְ� �׾ȿ� �߰��� Node*�� ����� �۾������ؾ��ؼ�
//// �ι� �ϴ°� ���� �ȵ�
//// -> ������ �ȿ� ��� ���õ� �� ���ܳ־��!
//template<typename T>
//struct Node
//{
//	T data;
//	Node* node;
//};

// --------------------
// 1�� �õ� ( �̱� ������ ȯ�� )
// --------------------
/*
struct SListEntry
{
	SListEntry* next;
};

struct SListHeader
{
	SListEntry* next = nullptr;
};

// [node + data ] [ ] [ ]
// [Header]

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry); // stack�� Ǫ�ÿ� ����
SListEntry* PopEntrySList(SListHeader* header);
*/

// --------------------
// 2�� �õ� ( ��Ƽ ������ ȯ�� )
// --------------------
//struct SListEntry
//{
//	SListEntry* next;
//};
//
//struct SListHeader 
//{
//	SListEntry* next = nullptr;
//};
//
//// [node + data ] [ ] [ ]
//// [Header]
//
//void InitializeHead(SListHeader* header);
//void PushEntrySList(SListHeader* header, SListEntry* entry); // stack�� Ǫ�ÿ� ����
//SListEntry* PopEntrySList(SListHeader* header);

// --------------------
// 3�� �õ�
// --------------------
DECLSPEC_ALIGN(16) // ������ �޸𸮸� 16����Ʈ�� ������ �ش޶�� �����Ϸ��� ����
struct SListEntry
{
	SListEntry* next;
};

struct SListHeader
{
	SListHeader()
	{
		alignment = 0;
		region = 0;
	}

	union
	{
		struct
		{
			uint64 alignment;
			uint64 region;
		} DUMMYSTRUCTNAME;
		struct
		{
			uint64 depth : 16; // 16��Ʈ ���
			uint64 sequence : 48; // 48��Ʈ ���
			uint64 reserved : 4;
			uint64 next : 60;
		} HeaderX64;
	};
};

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry); // stack�� Ǫ�ÿ� ����
SListEntry* PopEntrySList(SListHeader* header);
