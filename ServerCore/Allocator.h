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
class StompAllocator
{
	enum { PAGE_SIZE = 0x1000 };
public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};