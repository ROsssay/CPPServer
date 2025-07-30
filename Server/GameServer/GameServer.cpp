#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>


mutex m;
queue<int32> q;
HANDLE handle;

// CV는 userLevel Object (커널 오브젝트 x)
condition_variable cv;

void Producer()
{
	while (true)
	{
		// 1) Lock을 잡고
		// 2) 공유 변수 값을 수정
		// 3) Lock을 풀고
		// 4) 조건변수 통해 다른 쓰레드에게 통지

		{
			unique_lock<mutex> lock(m); //1) 3)
			q.push(100);				//2)
		}

		cv.notify_one(); // wait중인 스레드가 있으면 딱 한개를 깨운다 4)

		//::SetEvent(handle);			// 4)
		//this_thread::sleep_for(10000ms);
	}
}

void Consumer()
{
	while (true)
	{
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() {return q.empty() == false; });
		// 1) Lock을 잡고
		// 2) 조건 확인
		// - 만족 => 빠져나와서 이어서 코드를 진행
		// - 만족x => Lock을 풀어주고 대기상태로 전환

		// 그런데 notify_one을 했으면 항상 조건식을 만족하는거 아닐까?
		// Spurious Wakeup(가짜 기상?)
		// notify_one할 때 lock을 잡고 있는 것이 아니기 때문

		// while (q.empty() == false) << 43번줄에서 체크하기때문에 굳이 체크 안해도 됨
		{
			int32 data = q.front();
			q.pop();
			cout << q.size() << endl;
		}
	}
}

// 온라인 게임 서버에서 이용할때
// 한쪽에서 패킷을 쏘고, 컨텐츠 큐에서 데이터를 꺼내서 사용해야 할때,
// 효율적으로 리소스를 관리하기 위해서

int main()
{
	// 커널 오브젝트
	// Usage Count
	// Signal(파란불) / Non-Signal(빨간불) << bool
	// Auto / Manual << bool


	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();
}