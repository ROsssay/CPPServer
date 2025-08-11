#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <future>

#include <windows.h>

atomic<bool> flag = false;

int main()
{
	flag = false;

	flag.store(true, memory_order::memory_order_seq_cst);

	bool val = flag.load(memory_order::memory_order_seq_cst);

	//이전 flag 값을 prev에 넣고, flag값을 수정
	{
		bool prev = flag.exchange(true);
		//bool prev = flag;
		//flag = true;
	}

	// CAS (Compare-And-Swap) 조건부 수정
	{
		bool expected = false;
		bool desired = true;

		flag.compare_exchange_strong(expected, desired);
		//의사코드 비교
		if (flag == expected)
		{
			//expected = flag;
			flag = desired;
			return true;
		}
		else
		{
			expected = flag;
			return false;
		}
		// CAS는 원자적으로 한번에 일어남

		while (true)
		{
			bool expected = false;
			bool desired = true;
			flag.compare_exchange_weak(expected, desired);
		}

		// 다른 쓰레드의 interruption을 받아서 중간에 실패할 수 있음 strong은 실패시 반복함(strong 버전이 좀 더 부하가 있는 버전임)
		// weak는 그래서 while문으로 무한반복을 시켜주는 것이 일반적임
		// 두 버전의 차이는 별로 없음 생각보다
	}
}