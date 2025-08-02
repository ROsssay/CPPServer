#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

#include <future>

// 오늘 주제 sd future 미래객체
// 활용도가 조금 덜어짐
// C++11에 추가됨

//int64 result;

void PromiseWorker(std::promise<string>&& promise)
{
	promise.set_value("secret messege");
}

void TaskWorker(std::packaged_task<int64(void)>&& task)
{
	task();
}

int64 Calculate()
{
	int64 sum = 0;

	for (int32 i = 0; i < 100'0000; i++)
		sum += i;

	//result = sum;

	return sum;
}

int main()
{
	/*// 동기 (synchronous) 실행
	int64 sum = Calculate();
	cout << sum << endl;

	// but Calculate가 오래걸리는 함수라면? 비동기 방식이 유용할때가 생김
	// 비동기 방식 << 공용 데이터를 넘겨받아야 하는 껄끄러움 + 비효율적인 상황이 생길수가 있음
	// + Lock을 걸어야 정상적으로 작동하는 공용 데이터 작업이 깔끔하지 않음

	// 항상 thread를 만들어서 무겁게 하지 말고 future(단기알바느낌)을 쓰자

	thread t(Calculate);

	// TODO

	t.join();*/

	/////////////////////////////////////////////////////////////////////////

	//int64 sum = Calculate();
	//cout << sum << endl;

	// std::future | 비동기 방식
	{
		// 1) deferred -> lazy evaluation 지연해서 실행하세요
		// 2) async -> 별도의 쓰레드를 만들어서 실행하세요 >> 새로운 쓰레드를 만들어서 병렬로 연산됨
		// 3) deferred | async -> 둘 중 알아서 골라주세요

		//std::future<int64> future = std::async(std::launch::async, Calculate); // Calculate를 async 방식으로 호출

		////Calculate가 끝났는지 궁금하면?
		//std::future_status status = future.wait_for(1ms);
		//if (status == future_status::ready) {

		//} future.wait()도 있음

		// 결과물을 당장 받을 필요가 없다고 한다면 기다릴 필요가 없음

		//int64 sum = future.get(); // .get()으로 결과물이 필요할때 뽑을 수 있음
		// int64 sum = Calculate()랑 다를게 없지않나?
	}


	/*class Knight
	{
	public:
		int64 GetHp() { return 100; }
	};*/

	//Knight knight;
	//std::future<int64> future = std::async(std::launch::async, &Knight::GetHp, knight); // knight.GetHp();
	//// 멤버함수도 똑같이 호출가능 (객체대상 가능)


	// 동기방식 main >> Cal >> ... 1차원적인 방식
	// 비동기방식 main >> C(요청) >> ... >> Cal(호출) 요청과 호출의 시점이 다름
	
	// XML 서버에서 future 사용할때가 있음

	/////////////////////////

	// std::promise
	{
		// 미래(std::future)에 결과물을 반환해줄꺼라 약속(std::promise) 해줘~(계약서)
		std::promise<string> promise;
		std::future<string> future = promise.get_future(); // promise와 future가 연결된 상태

		thread t(PromiseWorker, std::move(promise));
		string messege = future.get();		// get()으로 데이터를 추출하면 future는 비어버림. 여러번 x

		cout << messege << endl;

		t.join();
	}

	//std::packaged_task
	{
		std::packaged_task<int64(void)> task(Calculate);		// 결과물을 future 객체에 받을 수 있음/ 이것말곤 차이 x
		std::future<int64> future = task.get_future();

		std::thread t(TaskWorker, std::move(task));

		int64 sum = future.get();
		cout << sum << endl;

		t.join();
	}

	
}

// 결)
// mutex, condition_variable 까지 가지 않고 단순한 애들 처리할 수 있는
// 특히나, 한 번 발생하는 이벤트에 유용하다!
// 닭잡는데 소잡는 칼을 쓸 필요 없다!
// 1) async
// 원하는 함수를 비동기적으로 실행
// 2) promise
// 결과물을 promise를 통해 future로 받아줌
// 3) packaged_task
// 원하는 함수의 실행 결과를 packaged_task를 통해 future로 받아줌

// 실제로 본적 없음 사용하는걸
// async는 사용할 일이 생길수도?
// future 중요도 낮음☆★