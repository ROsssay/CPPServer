#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

class SpinLock
{
public:
	void lock()
	{
		// CAS (Compare And Swap)
		bool expected = false;
		bool desired = true;

		while (_locked.compare_exchange_strong(expected, desired) == false) // 처음에 기대하는 값이 expected
		{																	// 바뀌길 기대하는 값이 desired 딱알았으
			expected = false;

			//this_thread::sleep_for(std::chrono::milliseconds(100));
			this_thread::sleep_for(0ms);
			//this_thread::yield(); == 0ms
		}
	}

	void unlock()
	{
		//_locked = false;
		_locked.store(false);
	}
private:
	atomic<bool> _locked = false;
};

SpinLock spinLock;
mutex m;
int32 sum = 0;

void Add()
{
	for (int32 i = 0; i < 10'0000; i++)
	{
		lock_guard<SpinLock> guard(spinLock);
		sum++;
	}
}

void Sub()
{
	for (int32 i = 0; i < 10'0000; i++)
	{
		lock_guard<SpinLock> guard(spinLock);
		sum--;
	}
}

int main()
{

	thread t1(Add);
	thread t2(Sub);

	t1.join();
	t2.join();

	cout << sum << endl;
}