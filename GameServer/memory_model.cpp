#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

atomic<bool> ready;
int32 value;

void Producer()
{
	value = 10;

	// seq-cst
	ready.store(true, memory_order::memory_order_seq_cst); // 기본 옵션 ( 안 쓰면 이걸로 설정 )
	
	
	// acquire-release
	ready.store(true, memory_order::memory_order_release); // 이거 다음부터는 재배치 안일어나도록 제한함

}

void Consumer()
{
	// seq-cst
	while (ready.load(memory_order::memory_order_seq_cst) == false) // 기본 모드
		;

	// acquire-release
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



	// atomic memory model (정책)
	// 1 Sequentially Consistent (seq_cst)
	// 2) Acquire-Release (acquire, release)
	// 3) Relaxed ( relaxed )

	// 1) seq_cst ( 가장 엄격  = 컴파일러 최적화 여지 적음 = 직관적 ) -> 코드 재배치 등이 잘 안일어남 ( atomic이 웬만하면 이거로 동작함 )
	// - 가시성 문제 바로 해결! 코드 재배치 문제 바로 해결!


	// 2) acquire-release
	// - 딱 중간!
	// - release 명령 이전의 메모리 명령들이, 해당 명령 이후로 재배치 되는 것을 금지
	// - 그리고 acquire로 같은 변수를 읽는 쓰레드가 있다면 release 이전의 명령들이 -> acquire하는 순간에 관찰 가능하다 ( 가시성 보장 )
	// 가시성 : release후 acquire가 되면 100% 확률로 acquire 다음에 value에는 10이 들어있도록 보장


	// 3) relaxed ( 자유롭다 = 컴파일러 최적화 여지 많음 = 직관적이지 않음 ) -> 최적화로 인해 코드 재배치 등이 잘 일어남
	// - 너무나도 자유롭다!
	// - 코드 재배치도 멋대로 가능! 가시성 해결 NO!
	// - 가장 기본 조건 ( 동일 객체에 대한 동일 관전 순서만 보장 )
	// - 거의 사용할 일이 없다! -> 멀티스레드에서 아주 재앙적인 문제라서 쓸 일 없음

	// 인텔, AMD의 경우 atomic을 사용할 때는 애당초 순차적 일관성을 보장을 해서, seq_cst를 써도 별다른 차이가 없다. ( 가시성, 코드 재배치 알아서 해결! )
	// 하지만 ARM의 경우 꽤 차이가 있다.




	/*atomic */
	//atomic<bool> flag;
	//flag = false;

	//flag.is_lock_free(); // 락이없다(is_lock_free() == true면 원자적으로 동작하니까 락을 걸 필요가 없어서 true로 뜸)

	////flag = true;
	////flag.store(true); // 가독성을 위해 ( atomic일떄는 이렇게 넣자 )
	//flag.store(true, memory_order::memory_order_seq_cst); // 가독성을 위해 ( atomic일떄는 이렇게 넣자 )

	////bool val = flag;
	////bool val = flag.load();
	//bool val = flag.load(memory_order::memory_order_seq_cst);

	//// 이전 flag 값을 prev에 넣고, flag값을 수정
	//{
	//	bool prev = flag.exchange(true);
	//	
	//	// 원자적 성질이라면 이렇게 가져오고 값을 갱신하는 동작이 한번에 이뤄져야 해서 flag.exchange()로 한다.
	//	/*bool prev = flag;
	//	flag = true;*/
	//}

	//// CAS ( compare and swap )
	//{
	//	bool expected = false;
	//	bool desired = true;
	//	flag.compare_exchange_strong(expected, desired);
	//	/*
	//	이게 CAS 의 의사코드
	//	if (flag == expected)
	//	{
	//		flag = desired;
	//		return true;
	//	}
	//	else
	//	{
	//		expected = flag;
	//		return false;
	//	}
	//	*/
	//	bool expected = false;
	//	bool desired = true;
	//	flag.compare_exchange_weak(expected, desired);
	//}
}