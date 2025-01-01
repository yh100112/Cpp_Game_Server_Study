#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0); // ó�� CP ����
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

// CP�� ������ ���(IocpObject)�� ���
bool IocpCore::Register(IocpObjectRef iocpObject)
{
	// Ciocp()�� �� session�� �ִ� �뵵�θ� ����� �� �ִ� �� �ƴϴ�.
	// �� ��Ʈ��ũ ����(socket, session..) ���� �ƴϴ��� �پ��� �뵵�� ����� �� �ֱ� ������
	// �װ� �������ϰ� ������� �� �ֵ��� IocpObject�� ���� �����ϱ�� ����!
	// Ciocp()�� ���� �� �ִ� ���� iocpObject�� ��ӹ޾Ƽ� ���� �ֵ��� ����
	// ù��° ���� : ������ TCP ������ ��ȯ��
	// ����° ���� : IocpObject�� �ϴ� ���� session�� ���� �����̹Ƿ� �ش� ������ �ּ� ���� Ű�� �־��ش�.
	//return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 
	//	reinterpret_cast<ULONG_PTR>(iocpObject), 0);
	
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, /*key*/0, 0); // �Ҹ�� ���ǿ� �����ϴ� ������ �ذ��ϱ� ���� key�� ������� �ʵ��� �� ( �ٸ� ������� ó�� )
}

// worker thread���� �ϰ��� �ִ��� �θ����θ��� ã�´�.
bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0; // �۽� Ȥ�� ���ŵ� ����Ʈ ũ��
	ULONG_PTR key = 0; 
	//IocpObject* iocpObject = nullptr; // ������ ���� (key��)
	IocpEvent* iocpEvent = nullptr; // ������ ���� (overlapped ����ü)

	// �� ������ iocpOjbect�� iocpEvent�� �Ҹ���� ���� ������ �ּ��̵��� �����ֱ� ���� key�� ������� �ʵ��� �����Ѵ�.

	// �����ϸ� �ΰ��� ������ �־ ���´� iocpObject�� iocpEvent ������ ��������
	// �츮�� �ϲ�(worker thread)���� ���⿡�� ���� ������ ��⸦ Ż ����
	if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT &key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		// �ϰ��� �־ 1�� �����尡 ���� ���� ���̹Ƿ� iocpObject->Dispatch�� ����
		// Listener Ŭ������ IocpOjbect�� ��ӹ޾Ƽ� Dispatch�� �������̵��ϹǷ� Listener::Dispatch()�� ȣ��
		IocpObjectRef iocpObject = iocpEvent->owner; // �ʸ� ���� �ִ� ���� ���δ��� ������?
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
			// TODO : �α� ���
			IocpObjectRef iocpObject = iocpEvent->owner; // �ʸ� ���� �ִ� ���� ���δ��� ������?
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}

	return true;
}
