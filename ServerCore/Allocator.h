#pragma once
// �Ҵ� ��å�� ����

/*------------
Base Allocator
------------*/
class BaseAllocator
{
public:
	static void* Alloc(int32 size); // �޸� �Ҵ�
	static void Release(void* ptr); // �޸� ����
};

/*------------
Stomp Allocator
-> ������ ����� ���׸� ��µ� ������
------------*/