#pragma once

// 정책
// 메모리 풀을 여러개 만들거임
// [32바이트 이하 데이터들 관리하는 메모리 풀][32 초과 64 이하 관리..][ ][ ][][][][]
// [                   ] 
// -> 하나의 메모리 풀 안에서 어디까지는 32, 어디까지는 64만 이런 식으로도 가능하긴함
/*---------------
	MemoryHeader
----------------*/
struct MemoryHeader
{
	// [MemoryHeader][Data]
	MemoryHeader(int32 size) : allocSize(size) { }
	
	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		new(header)MemoryHeader(size); // placement new
		return reinterpret_cast<void*>(++header); 
		// ++로 1일 더해주면 포인터 연산 특성상 메모리 헤더 만큼 건너뛰게 되므로
		// 정확히 Data의 시작 위치를 반환해줌
		// -> [MemoryHeader][Data]
		// -> 한 블록 건너뜀
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		// data 부분을 가리키는 포인터를 한칸 뒤로해서 memoryheader를 가리킴
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	int32 allocSize;
};

/*---------------
	MemoryPool
----------------*/
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void Push(MemoryHeader* ptr); // 메모리 풀에 다 쓴 메모리를 반납
	MemoryHeader* Pop(); // 메모리 풀에서 써야 할 메모리를 가져옴

private:
	int32 _allocSize = 0;
	atomic<int32> _allocCount = 0;

	USE_LOCK;
	queue<MemoryHeader*> _queue;
};

