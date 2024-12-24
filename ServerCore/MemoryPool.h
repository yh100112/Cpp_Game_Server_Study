#pragma once

// ��å
// �޸� Ǯ�� ������ �������
// [32����Ʈ ���� �����͵� �����ϴ� �޸� Ǯ][32 �ʰ� 64 ���� ����..][ ][ ][][][][]
// [                   ] 
// -> �ϳ��� �޸� Ǯ �ȿ��� �������� 32, �������� 64�� �̷� �����ε� �����ϱ���
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
		// ++�� 1�� �����ָ� ������ ���� Ư���� �޸� ��� ��ŭ �ǳʶٰ� �ǹǷ�
		// ��Ȯ�� Data�� ���� ��ġ�� ��ȯ����
		// -> [MemoryHeader][Data]
		// -> �� ��� �ǳʶ�
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		// data �κ��� ����Ű�� �����͸� ��ĭ �ڷ��ؼ� memoryheader�� ����Ŵ
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

	void Push(MemoryHeader* ptr); // �޸� Ǯ�� �� �� �޸𸮸� �ݳ�
	MemoryHeader* Pop(); // �޸� Ǯ���� ��� �� �޸𸮸� ������

private:
	int32 _allocSize = 0;
	atomic<int32> _allocCount = 0;

	USE_LOCK;
	queue<MemoryHeader*> _queue;
};

