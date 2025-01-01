#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket(); // tcp ���� �� �� ����
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registerSend = false; // ���ø޸𸮿� ���� ������ ���� ����

	// ���� RegisterSend�� �ɸ��� ���� ���¶��, �ɾ��ش�.
	{
		WRITE_LOCK;

		_sendQueue.push(sendBuffer); // ���� �����͸� �׾Ƴ���

		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}

	if (registerSend)
		RegisterSend();
}

// �������� ������ ���� �� ������ ���� ������ �ǹ�
// �̰� ���� ���� ������ �پ�� ��
bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	// �� ���� ȣ��ǵ��� ������
	if (_connected.exchange(false) == false)
		return;

	// TEMP
	wcout << "Disconnect : " << cause << endl;

	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

// iocpEvent�� recv�� send �̺�Ʈ�� ���� �̰� ó���� �� �� �κ����� ���ͼ� ó������
void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::DisConnect:
		ProcessDisConnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(numOfBytes);
		break;
	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != ServiceType::Client)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket, 0/*���°� �ƹ��ų� ���� ���� ����*/) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this(); // ADD_REF

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();
	if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr; // RELEASE_REF
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this(); // ADD_REF

	if (SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0) == false)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.owner = nullptr; // RELEASE_REF
			return false;
		}
	}

	return true;
}

// Listener���� Ŀ�ؼǱ��� �����µ� ���������� Ŀ�ؼ��� �ξ��
void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr; // RELEASE_REF

	_connected.store(true);

	// ���񽺿� ���� ���
	GetService()->AddSession(GetSessionRef());

	// ������ �ڵ忡�� �����ε�
	OnConnected();

	// ���� ��� ( ó�� �� �� ������ ȣ�� �ʿ�! - ���˴븦 ���� ���� ���� �� )
	RegisterRecv();
}

void Session::ProcessDisConnect()
{
	_disconnectEvent.owner = nullptr; // RELEASE_REF

	OnDisconnected(); // ������ �ڵ忡�� ������
	GetService()->ReleaseSession(GetSessionRef());
}

// recv�� ��Ƽ�����带 ������� �ʾƵ� ��
// ���������� �� ���� �� �����常 registerrecv(), processrecv()�� ����
// recv ������ �����̶� ������ �Ϸ� ������ IocpCore::Dispatch()�� GetQueueCompletionStatus�� �� ����
// �׷��� Dispatch�� Ÿ�� ProcessRevc���� �� ����
// ���⼭ �� ���� ���������� ���� ���˴븦 ������ ���� ����� ���� (RegisterRecv)
// -> �׷��Ƿ� ���˴�� 1���̹Ƿ� 1���� �����常 ������ �� �־ ��Ƽ������ ������ ����
void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this(); // ADD_REF

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos()); // w Ŀ�� ��ġ�� �����͸� �᳻����
	wsaBuf.len = _recvBuffer.FreeSize(); // �ִ�� ���� �� �ִ� ũ�Ⱑ �������� �ǹ�

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	// ���⸦ ����ϸ� ProcessRecv()�� ����
	if (::WSARecv(_socket, &wsaBuf, 1, OUT &numOfBytes, OUT &flags, &_recvEvent, nullptr) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_recvEvent.owner = nullptr; // RELEASE_REF
		}
	}
}

void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	_sendEvent.Init();
	_sendEvent.owner = shared_from_this(); // ADD_REF

	// ���� �����͸� sendEvent�� ���
	{
		// �̹� ���� ��� RegisterSend()�� �����ߴµ� �� �� ����?
		// ���߿� RegisterSend()���� ���� �� ��� �ϴ� �ɷ� �ٲ� �� �־ ���� �������� ����
		WRITE_LOCK;

		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer); // sendEvent�� ���
		}
	}

	// ������� �ٽ� �����ؼ� ��Ƽ�����带 ������� �ʾƵ� ��
	// �ִ��� RegisterSend�� �� ���� �� �����常 Ÿ�Ƿ�

	// Scatter-Gather ( ����� �ִ� �����͵��� ��Ƽ� �� �濡 ������ ��� )
	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (SendBufferRef sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	// iocp�� ����������� ��ϵǾ� �־ �Ϸᰡ �Ǹ� ProcessSend()�� �Ѿ�� �ȴ�.
	if (::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &_sendEvent, nullptr) == SOCKET_ERROR)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_sendEvent.owner = nullptr; // RELEASE_REF
			_sendEvent.sendBuffers.clear(); // RELEASE_REF
			_sendRegistered.store(false);
		}
	}
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr; // RELEASE_REF

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	// ���� ���� �� ���������� �츮�� ���ۿ� �����Ͱ� ����Ǿ��ٴ� ��

	// numOfBytes��ŭ w Ŀ���� ������ �̵���Ŵ
	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize();

	// ������ �ʿ��� ���� ������ �� ó���� ������ ���̸� ��ȯ����
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize); // ������ �ڵ忡�� ������

	// ������ �ʿ��� ó���� ������ ���̸�ŭ r Ŀ���� ������ �̵���Ŵ
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	// Ŀ�� ����
	_recvBuffer.Clean();

	// ���� ��� ( ������ ó���ǰ� ��� �۾��� �������Ƿ� �ٽ� ���ô븦 ������ ���� ������ ���� �غ��� )
	RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
	_sendEvent.owner = nullptr; // RELEASE_REF
	_sendEvent.sendBuffers.clear(); // RELEASE_REF

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	// ������ �ڵ忡�� ������
	OnSend(numOfBytes);

	WRITE_LOCK; // sendQueue ������ �� �ɾ�� ��
	if (_sendQueue.empty())
		_sendRegistered.store(false);
	else
		RegisterSend();
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		// TODO : Log
		cout << "Handle Error : " << errorCode << endl;
		break;
	}
}

/*-------------
	PacketSession
-------------*/
PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

// [size(2byte)][id(2byte)][data.....][size(2byte)][id(2byte)][data.....]
// size = data + PacketHeader�� �־���
// ����ü�� ������ ��Ŷ�� �Դٰ� Ȯ���� �� �ֵ��� ��Ŷ�� �������ִ� �Լ�
int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
	int32 processLen = 0;

	// packet 1���� ��� ó�� : [size(2byte)][id(2byte)][data.....]
	while (true)
	{
		int32 dataSize = len - processLen;
		// �ּ��� ����� �Ľ��� �� �־�� �Ѵ� ( 4byte )
		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		
		// ����� ��ϵ� ��Ŷ ũ�⸦ �Ľ��� �� �־�� �Ѵ�
		if (dataSize < header.size)
			break;

		// ��Ŷ ���� ����
		OnRecvPacket(&buffer[0], header.size);

		processLen += header.size;
	}

	return processLen;
}
