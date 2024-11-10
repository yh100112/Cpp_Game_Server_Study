#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

Session::Session()
{
	_socket = SocketUtils::CreateSocket(); // tcp 소켓 한 개 만듬
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	// TODO
	// iocpEvent가 recv나 send 이벤트를 만들어서 이걸 처리할 때 이 부분으로 들어와서 처리해줌
}
