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

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);

		if (_stack.empty())
			return false;

		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	void WaitPop(T& value)	
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
//
//template<typename T>
//class LockFreeStack
//{
//	struct Node
//	{
//		Node(const T& value) : data(make_shared<T>(value)), next(nullptr)
//		{
//
//		}
//
//		shared_ptr<T> data;
//		shared_ptr<Node> next;
//	};
//
//public:
//	void Push(const T& value)
//	{
//		shared_ptr<Node> node = make_shared<Node>(value);		// 문제 shared_ptr가 lock-free 방식으로 작동하지 않음 LockFreeStack은 거짓말이된다
//		node->next = std::atomic_load(&_head);
//		while (std::atomic_compare_exchange_weak(&_head, &node->next, node) == false)
//		{
//		}
//	}
//
//
//	shared_ptr<T> TryPop()
//	{
//		shared_ptr<Node> oldHead = std::atomic_load(&_head);
//
//		while (oldHead && std::atomic_compare_exchange_weak(&_head, &oldHead, oldHead->next) == false)
//		{
//
//		}
//
//		if (oldHead == nullptr)
//			return shared_ptr<T>();
//
//		return oldHead->data;
//		
//	}
//
//private:
//	shared_ptr<Node> _head;
//};

// 

template<typename T>
class LockFreeStack				// 콘큐런시 액션 책에서 나온 로직 ☆lock-free 프로그래밍은 니들 생각나는대로 하지마라
{
	struct Node;

	struct CountedNodePtr
	{
		int32 externalCount = 0;
		Node* ptr = nullptr;
	};

	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value))
		{

		}

		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next;
	};

public:
	void Push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;
		// [!]
		node.ptr->next = _head;
		while (_head.compare_exchange_weak(node.ptr->next, node) == false)
		{
		}
	}


	shared_ptr<T> TryPop()			// 정말로 lock-based 방식보다 빠른가? 경합이 붙었을때, 모든 로직을 다시 처음부터 실행해야 하는 경우가 빈번하게 생김.
									// Lock-free는 알고만 있어라 버려도 된다
	{
		CountedNodePtr oldHead = _head;
		while (true)
		{
			// 참조권 획득 (externalCount를 현 시점 기준 +1한 애가 이김)
			IncreaseHeadCount(oldHead);		// 은행 번호표 생각하면 쉬움
			// 최소한 externalCount >= 2 일테니 삭제X (안전하게 접근할 수 있는 상태)
			Node* ptr = oldHead.ptr;

			// 데이터 없음
			if (ptr == nullptr)
				return shared_ptr<T>();

			// 소유권 획득 (ptr->next로 head를 바꿔치기 한 애가 이김)
			if (_head.compare_exchange_strong(oldHead, ptr->next))
			{
				shared_ptr<T> res;
				res.swap(ptr->data);

				// 데이터를 꺼내 쓰는것까지는 좋았는데, 데이터를 꺼낸 Node를 삭제해도 되는가?

				// external : 1 -> 2(+1) -> 4(나+1 남+2)
				// internal : 1 -> 0

				// 나 말고 또 누가 있는가?
				const int32 countIncrease = oldHead.externalCount - 2;

				if (ptr->internalCount.fetch_add(countIncrease) == -countIncrease)
					delete ptr;

				return res;
			}
			else if(ptr->internalCount.fetch_sub(1) == 1)
			{
				// 참조권은 얻었으나, 소유권은 실패 -> 뒷수습은 내가한다.
				delete ptr;
			}
		}
	}

private:

	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true)
		{
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++;

			if (_head.compare_exchange_strong(oldCounter, newCounter))
			{
				oldCounter.externalCount = newCounter.externalCount;
				break;
			}


		}
	}

private:
	atomic<CountedNodePtr> _head;
};

