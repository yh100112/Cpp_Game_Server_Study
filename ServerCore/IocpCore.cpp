#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0); // 처음 CP 생성
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

// CP에 관찰할 대상(IocpObject)을 등록
bool IocpCore::Register(IocpObjectRef iocpObject)
{
	// Ciocp()가 꼭 session만 넣는 용도로만 사용할 수 있는 게 아니다.
	// 꼭 네트워크 정보(socket, session..) 등이 아니더라도 다양한 용도로 사용할 수 있기 때문에
	// 그걸 광범위하게 만들어줄 수 있도록 IocpObject로 만들어서 관리하기로 하자!
	// Ciocp()에 담을 수 있는 것은 iocpObject를 상속받아서 만들어서 넣도록 하자
	// 첫번째 인자 : 생성한 TCP 소켓을 반환함
	// 세번째 인자 : IocpObject가 하는 일이 session과 같은 역할이므로 해당 포인터 주소 값을 키로 넣어준다.
	//return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 
	//	reinterpret_cast<ULONG_PTR>(iocpObject), 0);
	
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, /*key*/0, 0); // 소멸된 세션에 접근하는 문제를 해결하기 위해 key를 사용하지 않도록 함 ( 다른 방식으로 처리 )
}

// worker thread들이 일감이 있는지 두리번두리번 찾는다.
bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0; // 송신 혹은 수신된 바이트 크기
	ULONG_PTR key = 0; 
	//IocpObject* iocpObject = nullptr; // 복원할 정보 (key값)
	IocpEvent* iocpEvent = nullptr; // 복원할 정보 (overlapped 구조체)

	// 이 순간에 iocpOjbect와 iocpEvent가 소멸되지 않은 안전한 주소이도록 막아주기 위해 key를 사용하지 않도록 수정한다.

	// 성공하면 두가지 정보를 넣어서 보냈던 iocpObject와 iocpEvent 정보를 복원해줌
	// 우리의 일꾼(worker thread)들이 여기에서 전부 무한정 대기를 탈 것임
	if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT &key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		// 일감이 있어서 1개 스레드가 깨서 들어온 것이므로 iocpObject->Dispatch를 실행
		// Listener 클래스가 IocpOjbect를 상속받아서 Dispatch를 오버라이딩하므로 Listener::Dispatch()를 호출
		IocpObjectRef iocpObject = iocpEvent->owner; // 너를 물고 있는 현재 주인님은 누구냐?
		iocpObject->Dispatch(iocpEvent, numOfBytes);
	}
	else
	{
		int32 errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			// TODO : 로그 찍기
			IocpObjectRef iocpObject = iocpEvent->owner; // 너를 물고 있는 현재 주인님은 누구냐?
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}

	return true;
}
