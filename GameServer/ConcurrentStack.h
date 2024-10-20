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
	
	// 멀티 스레드 환경에서 empty 체크와 pop을 한번에 할 수 있게 구현해야 한다.
	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_stack.empty())
			return false;

		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	// 멀티 스레드에서는 empty가 의미없음. empty를 체크하더라도 체크한 후 pop을 하기 직전 순간에 다른 스레드가 들어와서 pop 할 수 있기 때문
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
	// 1) 새 노드를 만들고
	// 2) 새 노드의 next = head
	// 3) head = 새 노드
	void Push(const T& value)
	{

		Node* node = new Node(value);
		node->next = _head;
		// 이 사이에 다른 스레드에게 새치기 다하면? ( head가 달라질 수 있음 )
		// _head = node;

		// CAS
		/*if (_head == node->next) // 중간에 다른 스레드가 들어와서 _head를 변경하지 않은 경우 stack의 맨 앞인 head를 현재 추가한 node로 변경
		{
			_head = node;
			return true;
		}
		else // 중간에 다른 스레드가 들어와서 _head가 변경된 경우 현재 추가한 노드의 다음을 head로 다시 바꾸고 루프 반복
		{
			node->next = _head;
			return false;
		}*/
		while (_head.compare_exchange_weak(node->next, node) == false) // atomic하게 체크와 푸시를 같이 해주는 함수
		{
		}
	}

	// 1) head읽기
	// 2) head->next 일기
	// 3) head = head->next ( 원래 있던 헤더의 다음 노드를 헤더로 바꿔줌 )
	// 4) data 추출해서 반환
	// 5) 추출한 노드를 삭제
	bool TryPop(T& value)
	{
		++_popCount;

		Node* oldHead = _head;

		 // CAS
		/*if (_head == oldHead) // 중간에 다른 스레드가 가로채지 않아서 oldHead가 현재 헤드인 경우 현재 헤드를 다음 노드로 바꾼 후 루프 종료
		{
			_head = oldHead->next;
			return true;
		}
		else { // 중간에 다른 스레드가 가로채서 oldHead의 값이 달라진 경우 oldHead를 현재 헤드로 바꾸고 루프 반복
			oldHead = _head;
			return false;
		}*/
		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
		}

		if (oldHead == nullptr) {
			--_popCount;
			return false;
		}

		value = oldHead->data; // oldHead를 팝!!
		// 잠시 삭제 보류 ( 멀티스레드에서 누군가가 동시에 해당 포인터를 참조할 경우 메모리를 날리면 참조 에러 발생할 수 있음 - 따로 해당 부분 처리 필요! )
		// delete oldHead;
		TryDelete(oldHead);
		return true;
	}

	void TryDelete(Node* oldHead)
	{
		// 나 외에 누가 있는지 체크
		if (_popCount == 1)
		{
			// 나 혼자네?
			
			// 이왕 혼자인거, 삭제 예약된 다른 데이터들도 삭제해보자
			Node* node = _pendingList.exchange(nullptr);

			if (--_popCount == 0)
			{
				// 끼어든 애가 없음 -> 삭제 진행
				// 이제와서 끼어들어도 어차피 데이터는 분리해둔 상태~!
				DeleteNodes(node);
			}
			else if (node) // null이 아니면
			{
				// 누가 끼어들었으니 다시 갖다 놓자
				ChainPendingNodeList(node);
			}

			// 내 데이터는 삭제
			delete oldHead;
		}
		else
		{
			// 누가 있네? 그럼 지금 삭제하지 않고, 삭제 예약만
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
	atomic<Node*> _head; // stack 맨 위

	atomic<uint32> _popCount = 0; // 현재 Pop을 실행중인 쓰레드 개수
	atomic<Node*> _pendingList; // 삭제 되어야 할 노드들 ( 첫번째 노드 )
};