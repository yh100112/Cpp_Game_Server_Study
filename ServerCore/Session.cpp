#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket(); // tcp 소켓 한 개 만듬
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registerSend = false; // 스택메모리에 내가 보내야 될지 여부

	// 현재 RegisterSend가 걸리지 않은 상태라면, 걸어준다.
	{
		WRITE_LOCK;

		_sendQueue.push(sendBuffer); // 보낼 데이터를 쌓아놓음

		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}

	if (registerSend)
		RegisterSend();
}

// 서버끼리 연결할 때는 이 세션이 상대방 서버를 의미
// 이걸 통해 상대방 서버에 붙어야 함
bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	// 한 번만 호출되도록 막아줌
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

// iocpEvent가 recv나 send 이벤트를 만들어서 이걸 처리할 때 이 부분으로 들어와서 처리해줌
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

	if (SocketUtils::BindAnyAddress(_socket, 0/*남는거 아무거나 전부 접속 가능*/) == false)
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

// Listener에서 커넥션까지 들어오는데 성공했으니 커넥션을 맺어라
void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr; // RELEASE_REF

	_connected.store(true);

	// 서비스에 세션 등록
	GetService()->AddSession(GetSessionRef());

	// 컨텐츠 코드에서 오버로딩
	OnConnected();

	// 수신 등록 ( 처음 한 번 무조건 호출 필요! - 낚싯대를 물에 던져 놓는 것 )
	RegisterRecv();
}

void Session::ProcessDisConnect()
{
	_disconnectEvent.owner = nullptr; // RELEASE_REF

	OnDisconnected(); // 컨텐츠 코드에서 재정의
	GetService()->ReleaseSession(GetSessionRef());
}

// recv는 멀티스레드를 고려하지 않아도 됨
// 실질적으로 한 번에 한 스레드만 registerrecv(), processrecv()에 들어옴
// recv 받을게 조금이라도 있으면 완료 통지가 IocpCore::Dispatch()의 GetQueueCompletionStatus로 올 것임
// 그러면 Dispatch를 타고 ProcessRevc까지 올 것임
// 여기서 다 끝난 다음에서야 다음 낚싯대를 던져서 다음 등록을 해줌 (RegisterRecv)
// -> 그러므로 낚싯대는 1개이므로 1개의 스레드만 실행할 수 있어서 멀티스레드 문제가 없음
void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this(); // ADD_REF

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos()); // w 커서 위치에 데이터를 써내려감
	wsaBuf.len = _recvBuffer.FreeSize(); // 최대로 받을 수 있는 크기가 얼마인지를 의미

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	// 여기를 통과하면 ProcessRecv()에 들어옴
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

	// 보낼 데이터를 sendEvent에 등록
	{
		// 이미 락을 잡고 RegisterSend()를 실행했는데 왜 또 잡지?
		// 나중에 RegisterSend()에서 락을 안 잡고 하는 걸로 바뀔 수 있어서 락을 이중으로 잡음
		WRITE_LOCK;

		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer); // sendEvent에 등록
		}
	}

	// 여기부턴 다시 안전해서 멀티스레드를 고려하지 않아도 됨
	// 애당초 RegisterSend는 한 번에 한 스레드만 타므로

	// Scatter-Gather ( 흩어져 있는 데이터들을 모아서 한 방에 보내는 기법 )
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
	// iocp에 관찰대상으로 등록되어 있어서 완료가 되면 ProcessSend()로 넘어가게 된다.
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

	// 여기 들어온 건 성공적으로 우리의 버퍼에 데이터가 복사되었다는 뜻

	// numOfBytes만큼 w 커서를 앞으로 이동시킴
	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize();

	// 컨텐츠 쪽에서 받은 데이터 중 처리한 데이터 길이를 반환해줌
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize); // 컨텐츠 코드에서 재정의

	// 컨텐츠 쪽에서 처리한 데이터 길이만큼 r 커서를 앞으로 이동시킴
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	// 커서 정리
	_recvBuffer.Clean();

	// 수신 등록 ( 연결이 처리되고 모든 작업이 끝났으므로 다시 낚시대를 던져서 다음 연결을 받을 준비함 )
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

	// 컨텐츠 코드에서 재정의
	OnSend(numOfBytes);

	WRITE_LOCK; // sendQueue 때문에 락 걸어야 함
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
// size = data + PacketHeader를 넣어줌
// 완전체로 조립된 패킷이 왔다고 확신할 수 있도록 패킷을 조립해주는 함수
int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
	int32 processLen = 0;

	// packet 1개씩 계속 처리 : [size(2byte)][id(2byte)][data.....]
	while (true)
	{
		int32 dataSize = len - processLen;
		// 최소한 헤더는 파싱할 수 있어야 한다 ( 4byte )
		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		
		// 헤더에 기록된 패킷 크기를 파싱할 수 있어야 한다
		if (dataSize < header.size)
			break;

		// 패킷 조립 성공
		OnRecvPacket(&buffer[0], header.size);

		processLen += header.size;
	}

	return processLen;
}
