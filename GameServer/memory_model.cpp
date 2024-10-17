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
	ready.store(true, memory_order::memory_order_seq_cst); // �⺻ �ɼ� ( �� ���� �̰ɷ� ���� )
	
	
	// acquire-release
	ready.store(true, memory_order::memory_order_release); // �̰� �������ʹ� ���ġ ���Ͼ���� ������

}

void Consumer()
{
	// seq-cst
	while (ready.load(memory_order::memory_order_seq_cst) == false) // �⺻ ���
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



	// atomic memory model (��å)
	// 1 Sequentially Consistent (seq_cst)
	// 2) Acquire-Release (acquire, release)
	// 3) Relaxed ( relaxed )

	// 1) seq_cst ( ���� ����  = �����Ϸ� ����ȭ ���� ���� = ������ ) -> �ڵ� ���ġ ���� �� ���Ͼ ( atomic�� �����ϸ� �̰ŷ� ������ )
	// - ���ü� ���� �ٷ� �ذ�! �ڵ� ���ġ ���� �ٷ� �ذ�!


	// 2) acquire-release
	// - �� �߰�!
	// - release ��� ������ �޸� ��ɵ���, �ش� ��� ���ķ� ���ġ �Ǵ� ���� ����
	// - �׸��� acquire�� ���� ������ �д� �����尡 �ִٸ� release ������ ��ɵ��� -> acquire�ϴ� ������ ���� �����ϴ� ( ���ü� ���� )
	// ���ü� : release�� acquire�� �Ǹ� 100% Ȯ���� acquire ������ value���� 10�� ����ֵ��� ����


	// 3) relaxed ( �����Ӵ� = �����Ϸ� ����ȭ ���� ���� = ���������� ���� ) -> ����ȭ�� ���� �ڵ� ���ġ ���� �� �Ͼ
	// - �ʹ����� �����Ӵ�!
	// - �ڵ� ���ġ�� �ڴ�� ����! ���ü� �ذ� NO!
	// - ���� �⺻ ���� ( ���� ��ü�� ���� ���� ���� ������ ���� )
	// - ���� ����� ���� ����! -> ��Ƽ�����忡�� ���� ������� ������ �� �� ����

	// ����, AMD�� ��� atomic�� ����� ���� �ִ��� ������ �ϰ����� ������ �ؼ�, seq_cst�� �ᵵ ���ٸ� ���̰� ����. ( ���ü�, �ڵ� ���ġ �˾Ƽ� �ذ�! )
	// ������ ARM�� ��� �� ���̰� �ִ�.




	/*atomic */
	//atomic<bool> flag;
	//flag = false;

	//flag.is_lock_free(); // ���̾���(is_lock_free() == true�� ���������� �����ϴϱ� ���� �� �ʿ䰡 ��� true�� ��)

	////flag = true;
	////flag.store(true); // �������� ���� ( atomic�ϋ��� �̷��� ���� )
	//flag.store(true, memory_order::memory_order_seq_cst); // �������� ���� ( atomic�ϋ��� �̷��� ���� )

	////bool val = flag;
	////bool val = flag.load();
	//bool val = flag.load(memory_order::memory_order_seq_cst);

	//// ���� flag ���� prev�� �ְ�, flag���� ����
	//{
	//	bool prev = flag.exchange(true);
	//	
	//	// ������ �����̶�� �̷��� �������� ���� �����ϴ� ������ �ѹ��� �̷����� �ؼ� flag.exchange()�� �Ѵ�.
	//	/*bool prev = flag;
	//	flag = true;*/
	//}

	//// CAS ( compare and swap )
	//{
	//	bool expected = false;
	//	bool desired = true;
	//	flag.compare_exchange_strong(expected, desired);
	//	/*
	//	�̰� CAS �� �ǻ��ڵ�
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