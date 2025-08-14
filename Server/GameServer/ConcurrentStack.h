#pragma once

#include <mutex>

template<typename T>

// std::move >> 복사 없이 데이터를 그대로 전달하는 방법

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

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);

		if (_stack.empty())
			return false;

		// 일반적 싱글스레드 : empty > top > pop

		value = std::move(_stack.top());
		_stack.pop();
		// C#이나 C에서는 Pop으로 바로 꺼내올 수 있는데
		// C++에서는 한번 엿보고 꺼내오는 방식을 씀. why?
		// 한번에 데이터를 꺼내온다고 하면, 데이터를 꺼내오는 순간에 exception이 일어날 수 있음.
		// stack, queue가 깨져버릴 수 있기 때문에, 위와 두단계를 걸치는 구조를 사용함
		// 사실 게임에서는 데이터를 꺼내다가 데이터가 고갈나서 크래쉬가 난다면 그냥 뻗게 냅두는게 더 좋다.
		return true;
	}

	void WaitPop(T& value)			// 우아하게 무한정 데이터를 기다리는게 아니라, 데이터가 있을 때만 사용할 수 있다. ☆waitpop
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] {return _stack.empty() == false; });
		value = std::move(_stack.top());
		_stack.pop();
	}

	/*bool Empty()
	{
		lock_guard<mutex> lock(_mutex);
		return _stack.empty();
	}*/


private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar;
};

