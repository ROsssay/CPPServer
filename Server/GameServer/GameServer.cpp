#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include <windows.h>

atomic<bool> ready;
int32 value;

void Producer()
{
	value = 10;

	ready.store(true);

	//std::atomic_thread_fence(memory_order_release);
	// -------------절취선---------------
}

void Consumer()
{
	// -------------절취선---------------
	while (ready.load(memory_order::memory_order_acquire) == false)
		;

	cout << value << endl;
}

int main()
{
	ready = false;
	value = 0;

	thread t1(Producer);
	thread t2(Consumer);
	t1.join();
	t2.join();

	// atomic의 Memory Model (정책) 크게 3가지로 분류됨
	// 1)Sequentially Consistent (seq_cst)
	// 2) Acquire-Release (consume, acquire, release, acq_rel)
	// 3) Relaxed (relaxed)

	// 1) seq_cst (가장 엄격 = 컴파일러 최적화 여지 적음 = 직관적)   아무것도 입력하지 않을시 atomic은 이 버전으로 작동함
	// 가시성 문제 바로 해결! 코드 재배치 바로 해결!
	
	// 2) acquire-relase
	// 딱 중간
	// 짝을 맞춰줘야함 (acq, rel)  producer에서 release, consumer에서 acquire
	// release 명령 이전의 메모리 명령들이, 해당 명령 이후로 재배치 되는 것을 금지
	// 그리고 acquire로 같은 변수를 읽는 쓰레드가 있다면
	// release 이전의 명령들이 -> acquire 하는 순간에 관찰 가능 (가시성 보장)
	// 가끔씩 사용할 수 있겠다.
	
	// 3) relaxed (자유롭다 = 컴파일러 최적화 여지 많음 = 직관적이지 않음 = 끔찍한 코드)
	// 너무나도 자유롭다!
	// 코드 재비치도 멋대로 가능! 가시성 해결 NO! << 그래서 거의 사용할 일이 없음
	// 가장 기본 조건 (동일 객체에 대한 동일 관전 순서만 보장)

	// 인텔, AMD의 경우 애당초 순차적인 일관성을 보장해서
	// seq_cst를 써도 별 차이가 없다.
	// 그래서 그냥 default로 써도 된다
	// 하지만 ARM의 경우 꽤 차이가 있다고 함
}