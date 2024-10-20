#pragma once

#include <mutex>

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

