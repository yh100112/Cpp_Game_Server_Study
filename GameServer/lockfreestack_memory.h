#pragma once

//// 데이터 하나를 만들기 위해 Node를 만들어 주고 그안에 추가로 Node*를 만드는 작업까지해야해서
//// 두번 하는게 맘에 안듬
//// -> 데이터 안에 노드 관련된 걸 낑겨넣어보자!
//template<typename T>
//struct Node
//{
//	T data;
//	Node* node;
//};

// --------------------
// 1차 시도 ( 싱글 스레드 환경 )
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
void PushEntrySList(SListHeader* header, SListEntry* entry); // stack의 푸시와 같음
SListEntry* PopEntrySList(SListHeader* header);
*/

// --------------------
// 2차 시도 ( 멀티 스레드 환경 )
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
//void PushEntrySList(SListHeader* header, SListEntry* entry); // stack의 푸시와 같음
//SListEntry* PopEntrySList(SListHeader* header);

// --------------------
// 3차 시도
// --------------------
DECLSPEC_ALIGN(16) // 무조건 메모리를 16바이트로 지정을 해달라고 컴파일러에 지정
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
			uint64 depth : 16; // 16비트 사용
			uint64 sequence : 48; // 48비트 사용
			uint64 reserved : 4;
			uint64 next : 60;
		} HeaderX64;
	};
};

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry); // stack의 푸시와 같음
SListEntry* PopEntrySList(SListHeader* header);
