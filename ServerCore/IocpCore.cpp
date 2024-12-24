#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

// temp
IocpCore GlocpCore;

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
bool IocpCore::Register(IocpObject* iocpObject)
{
	// SOCKET socket;
	//return ::CreateIoCompletionPort(socket, _iocpHandle, /*key*/0, 0);

	// Ciocp()�� �� session�� �ִ� �뵵�θ� ����� �� �ִ� �� �ƴϴ�.
	// �� ��Ʈ��ũ ����(socket, session..) ���� �ƴϴ��� �پ��� �뵵�� ����� �� �ֱ� ������
	// �װ� �������ϰ� ������� �� �ֵ��� IocpObject�� ���� �����ϱ�� ����!
	// Ciocp()�� ���� �� �ִ� ���� iocpObject�� ��ӹ޾Ƽ� ���� �ֵ��� ����
	// ù��° ���� : ������ TCP ������ ��ȯ��
	// ����° ���� : IocpObject�� �ϴ� ���� session�� ���� �����̹Ƿ� �ش� ������ �ּ� ���� Ű�� �־��ش�.
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 
		reinterpret_cast<ULONG_PTR>(iocpObject), 0);
}

// worker thread���� �ϰ��� �ִ��� �θ����θ��� ã�´�.
bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0; // �۽� Ȥ�� ���ŵ� ����Ʈ ũ��
	IocpObject* iocpObject = nullptr; // ������ ����
	IocpEvent* iocpEvent = nullptr; // ������ ����

	// �����ϸ� �ΰ��� ������ �־ ���´� iocpObject�� iocpEvent ������ ��������
	// �츮�� �ϲ�(worker thread)���� ���⿡�� ���� ������ ��⸦ Ż ����
	if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT reinterpret_cast<PULONG_PTR>(&iocpObject), 
		OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		// �ϰ��� �־ 1�� �����尡 ���� ���� ���̹Ƿ� iocpObject->Dispatch�� ����
		// Listener Ŭ������ IocpOjbect�� ��ӹ޾Ƽ� Dispatch�� �������̵��ϹǷ� Listener::Dispatch()�� ȣ��
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
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}

	return true;
}
