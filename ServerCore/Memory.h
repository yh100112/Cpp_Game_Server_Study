#pragma once
#include "Allocator.h"

// allocator.h(class BaseAllocator)에서 정의한 new를 다른 방식으로 new를 만듬
// ... : 인자 개수가 가변적임
template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(Xalloc(sizeof(Type)));

	// 메모리가 먼저 할당된 상태에서 그 메모리 위에 어떤 객체의 생성자를 호출하는 문법
	// placement new라는 문법
	// - 메모리는 이미 있으니까 메모리를 새로 생성하지 말고 이 메모리 위에 생성자를 호출해줘~
	// - 여러 가변적인 개수의 인자 args를 생성자에 넘겨줌 ( forward<Args>() )
	new(memory)Type(std::forward<Args>(args)...);

	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	Xrelease(obj);
}
