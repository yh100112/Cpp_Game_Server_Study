#pragma once
#include "Allocator.h"

// allocator.h(class BaseAllocator)���� ������ new�� �ٸ� ������� new�� ����
// ... : ���� ������ ��������
template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(Xalloc(sizeof(Type)));

	// �޸𸮰� ���� �Ҵ�� ���¿��� �� �޸� ���� � ��ü�� �����ڸ� ȣ���ϴ� ����
	// placement new��� ����
	// - �޸𸮴� �̹� �����ϱ� �޸𸮸� ���� �������� ���� �� �޸� ���� �����ڸ� ȣ������~
	// - ���� �������� ������ ���� args�� �����ڿ� �Ѱ��� ( forward<Args>() )
	new(memory)Type(std::forward<Args>(args)...);

	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	Xrelease(obj);
}
