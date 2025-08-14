#pragma once

#include <mutex>
#include <atomic>

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

template<typename T>
class LockFreeStack
{
	struct Node
	{
		Node(const T& value) : data(value)
		{

		}

		T data;
		Node* next;
	};

public:
	
	// 1) 새 노드를 만들고
	// 2) 새 노드의 NEXT = HEAD
	// 3) head = 새 노드

	// [ ][ ][ ][ ][ ][ ][ ][ ]
	// [head]
	void Push(const T& value)
	{
		Node* node = new Node(value);
		node->next = _head;

		while (_head.compare_exchange_weak(node->next, node) == false)
		{

		}


		// 이 사이에 새치기 당하면?
		// _head = node;
	}


	// 1) head 읽기
	// 2) head->next 읽기
	// 3) head = head->next
	// 4) data 추출해서 반환
	// 5) 추출한 노드 삭제

	bool TryPop(T& value)
	{
		++_popCount;

		Node* oldHead = _head;

		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{

		}

		if (oldHead == nullptr)
		{
			--_popCount;
			return false;
		}

		value = oldHead->data;

		TryDelete(oldHead);

		return true;
	}

	// 1) 데이터 분리
	// 2) Count 체크
	// 3) 나 혼자면 삭제
	void TryDelete(Node* oldHead)
	{
		// 나 외에 누가 있는가?

		if (_popCount == 1)
		{
			// 나 혼자네?

			// 이왕 혼자인거, 삭제 예약된 다른 데이터들도 삭제해보자
			Node* node = _pendingList.exchange(nullptr);

			if (--_popCount == 0) // popCount 자체가 atomic이기 때문에 atomic하게 연산이 일어나서 괜찮음
			{
				// 끼어든 애가 없음 -> 삭제 진행
				// 이제와서 끼어들어도, 어차피 데이터는 분리해둔 상태~!
				DeleteNodes(node);
			}
			else if (node)
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

	// [ ][ ][ ][ ][ ][ ][ ] -> [ ] [ ] [ ] [ ]

	// [ ] [ ] [ ] [ ]
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
	atomic<Node*> _head;

	atomic<uint32> _popCount = 0; // Pop을 실행중인 쓰레드 개수
	atomic<Node*> _pendingList; // 삭제 되어야 할 노드들 (첫번째 노드)
};

