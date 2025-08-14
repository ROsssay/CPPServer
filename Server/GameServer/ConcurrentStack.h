#pragma once

#include <mutex>

template<typename T>

// std::move >> ���� ���� �����͸� �״�� �����ϴ� ���

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

		// �Ϲ��� �̱۽����� : empty > top > pop

		value = std::move(_stack.top());
		_stack.pop();
		// C#�̳� C������ Pop���� �ٷ� ������ �� �ִµ�
		// C++������ �ѹ� ������ �������� ����� ��. why?
		// �ѹ��� �����͸� �����´ٰ� �ϸ�, �����͸� �������� ������ exception�� �Ͼ �� ����.
		// stack, queue�� �������� �� �ֱ� ������, ���� �δܰ踦 ��ġ�� ������ �����
		// ��� ���ӿ����� �����͸� �����ٰ� �����Ͱ� ������ ũ������ ���ٸ� �׳� ���� ���δ°� �� ����.
		return true;
	}

	void WaitPop(T& value)			// ����ϰ� ������ �����͸� ��ٸ��°� �ƴ϶�, �����Ͱ� ���� ���� ����� �� �ִ�. ��waitpop
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

