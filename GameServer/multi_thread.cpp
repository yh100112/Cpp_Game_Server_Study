#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>

void HelloThread()
{
    cout << "Hello Thread" << endl;
}

void HelloThread2(int32 num)
{
    cout << num << endl;
}

// windows, linux 서버 둘 다에서 공용으로 사용되는 라이브러리를 사용하도록 하자!
int main()
{
    vector<std::thread> v;

    for (int32 i = 0; i < 10; i++)
    {
        v.push_back(std::thread(HelloThread2, i));
    }

    for (int32 i = 0; i < 10; i++)
    {
        if (v[i].joinable())
            v[i].join();
    }


    cout << "Hello Main" << endl;
}