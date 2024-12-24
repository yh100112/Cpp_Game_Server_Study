#pragma once

#include <mutex>
#include <atomic>

template<typename T>
class LockStack
{
public:
	LockStack() { }

	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move(value));
		_condVar.notify_one();
	}
	
	// ��Ƽ ������ ȯ�濡�� empty üũ�� pop�� �ѹ��� �� �� �ְ� �����ؾ� �Ѵ�.
	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_stack.empty())
			return false;

		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	// ��Ƽ �����忡���� empty�� �ǹ̾���. empty�� üũ�ϴ��� üũ�� �� pop�� �ϱ� ���� ������ �ٸ� �����尡 ���ͼ� pop �� �� �ֱ� ����
	//bool Empty()
	//{
	//	lock_guard<mutex> lock(_mutex);
	//	return _stack.empty();
	//}

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] { return _stack.empty() == false;  });
		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar;
};

// LockFreeStack�� ���� �� ��´ٴ°� �ƴ�. ��������� �� ���� ������ CAS�� ó����
// ���� ���� ���ڸ� � �����尡 ��� Ʈ�����ص� �ٸ� �����忡 ���� ��� CAS���� ���̺� ��(������̶�� ���� ����)
// ���·� �Ѿ�� ���ϰ� �ȴ�.
template<typename T>
class LockFreeStack
{
	struct Node
	{
		Node(const T& value) : data(value), next(nullptr) { }

		T data;
		Node* next;
	};

public:
	// 1) �� ��带 �����
	// 2) �� ����� next = head
	// 3) head = �� ���
	void Push(const T& value)
	{

		Node* node = new Node(value);
		node->next = _head;
		// �� ���̿� �ٸ� �����忡�� ��ġ�� ���ϸ�? ( head�� �޶��� �� ���� )
		// _head = node;

		// CAS
		// �߰��� �ٸ� ������ ������ _head�� ������� ���� ��� stack �� �� head�� ���� �߰��� node�� ����
		/*if (_head == node->next) 
		{
			_head = node;
			return true;
		}
		// �߰��� �ٸ� ������ ������ _head�� ����� ��� ���� �߰��� ����� ������ head�� �ٽ� �ٲٰ� ���� �ݺ�
		else 
		{
			node->next = _head;
			return false;
		}*/
		// _head�� expected(node-next)�� ������ ��� ����, �ٸ��� ó�����·� ������ ���ѷ���
		// atomic�ϰ� üũ�� Ǫ�ø� ���� ���ִ� �Լ�
		while (_head.compare_exchange_weak(node->next, node) == false) {}
	}

	// 1) head�б�
	// 2) head->next �ϱ�
	// 3) head = head->next ( ���� �ִ� ����� ���� ��带 ����� �ٲ��� )
	// 4) data �����ؼ� ��ȯ
	// 5) ������ ��带 ����
	bool TryPop(T& value)
	{
		++_popCount;

		Node* oldHead = _head;

		// CAS
		// �߰��� �ٸ� �����尡 ����ä�� �ʾƼ� oldHead�� ���� ����� ��� ���� ��带 ���� ���� �ٲ� �� ���� ����
		/*if (_head == oldHead) 
		{
			_head = oldHead->next;
			return true;
		}
		// �߰��� �ٸ� �����尡 ����ä�� oldHead�� ���� �޶��� ��� oldHead�� ���� ���� �ٲٰ� ���� �ݺ�
		else {
			oldHead = _head;
			return false;
		}*/
		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false) {}

		if (oldHead == nullptr) {
			--_popCount;
			return false;
		}

		value = oldHead->data; // oldHead�� pop

		
		// delete oldHead;
		// �̷��� �����ϸ� ���� �߻� 
		// - ��Ƽ�����忡�� ���� ���ÿ� �ش� �����͸� ������ ��� 
		//   �޸𸮸� ������ ���� ���� �߻��� �� ���� - ���� �ش� �κ� ó�� �ʿ�!
		// C#�̳� java��� �޸� ������ ������ �˾Ƽ� �Ͼ�� �Ű浵 �Ƚᵵ ������ c++�� �ٸ���.

		TryDelete(oldHead);

		return true;
	}

	void TryDelete(Node* oldHead)
	{
		// �� �ܿ� ���� �ִ��� üũ
		if (_popCount == 1)
		{
			// �� ȥ�ڳ�?
			
			// �̿� ȥ���ΰ�, ���� ����� �ٸ� �����͵鵵 �����غ���
			Node* node = _pendingList.exchange(nullptr);

			if (--_popCount == 0)
			{
				// ����� �ְ� ���� -> ���� ����
				// �����ͼ� ����� ������ �����ʹ� �и��ص� ����~!
				DeleteNodes(node);
			}
			else if (node) // null�� �ƴϸ�
			{
				// ���� ���������� �ٽ� ���� ����
				ChainPendingNodeList(node);
			}

			// �� �����ʹ� ����
			delete oldHead;
		}
		else
		{
			// ���� �ֳ�? �׷� ���� �������� �ʰ�, ���� ���ุ
			ChainPendingNode(oldHead);
			--_popCount;
		}
	}

	
	void ChainPendingNodeList(Node* first, Node* last)
	{
		last->next = _pendingList;

		while (_pendingList.compare_exchange_weak(last->next, first) == false)
		{
		}
	}

	void ChainPendingNodeList(Node* node)
	{
		Node* last = node;
		while (last->next)
			last = last->next;

		ChainPendingNodeList(node, last);
	}

	void ChainPendingNode(Node* node)
	{
		ChainPendingNodeList(node, node);
	}

	static void DeleteNodes(Node* node)
	{
		while (node)
		{
			Node* next = node->next;
			delete node;
			node = next;
		}
	}
private:
	atomic<Node*> _head; // stack �� ��

	atomic<uint32> _popCount = 0; // ���� Pop�� �������� ������ ����
	atomic<Node*> _pendingList; // ���� �Ǿ�� �� ���� ( ù��° ��� )
};