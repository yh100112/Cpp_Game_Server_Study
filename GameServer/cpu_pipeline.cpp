#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>


// 문제 : 가시성, 코드 재배치
int32 x = 0;
int32 y = 0;
int32 r1 = 0;
int32 r2 = 0;

volatile bool ready;

void Thread_1()
{
	while (!ready);

	y = 1; // store y
	r1 = x; // load x
}

void Thread_2()
{
	while (!ready);

	x = 1; // store x
	r2 = y; // load y
}

int main() {
	int32 count = 0;

	while (true) {
		ready = false;
		count++;

		x =	y = r1 = r2 = 0;

		thread t1(Thread_1);
		thread t2(Thread_2);

		ready = true;

		t1.join();
		t2.join();

		if (r1 == 0 && r2 == 0)
			break;
	}

	cout << count << " 번 만에 빠져나옴~" << endl;
}