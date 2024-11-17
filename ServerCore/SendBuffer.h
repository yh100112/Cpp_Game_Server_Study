#pragma once

class SendBufferChunk;

/*-----------------
	SendBuffer
-----------------*/
class SendBuffer
{
public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, int32 allocSize);
	~SendBuffer();

	BYTE*		Buffer() { return _buffer; }
	int32		WriteSize() { return _writeSize; }
	void		Close(uint32 writeSize);

private:
	BYTE*					_buffer;		// sendbufferchunk를 사용하므로 포인터만 들고있으면 됨
	uint32					_allocSize = 0;
	uint32					_writeSize = 0; // 실제 write에 사용된 버퍼 사이즈
	SendBufferChunkRef		_owner;			// 내가 참조하고 있으면 너는 절대 사라지면 안돼!
};


/*-----------------
	SendBufferChunk ( 굉장히 큰 버퍼 한개를 생성하고 필요할 때마다 쪼개서 사용하는 정책 )
-----------------*/
class SendBufferChunk : public enable_shared_from_this<SendBufferChunk>
{
	enum
	{
		SEND_BUFFER_CHUNK_SIZE = 6000
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

	void				Reset();
	SendBufferRef		Open(uint32 allocSize); // 열어줄 공간 ( 할당할 공간 )
	void				Close(uint32 writeSize); // 실질적으로 사용한 공간 = writeSize

	bool				IsOpen() { return _open; }
	BYTE*				Buffer() { return &_buffer[_usedSize]; } // 사용한 곳 다음 위치의 주소를 반환
	uint32				FreeSize() { return static_cast<uint32>(_buffer.size()) - _usedSize; }

private:
	Array<BYTE, SEND_BUFFER_CHUNK_SIZE>		_buffer = {};
	bool									_open = false;
	uint32									_usedSize = 0;
};

/*-----------------
	SendBufferManager
-----------------*/
class SendBufferManager
{
public:
	SendBufferRef Open(uint32 size); // 굉장히 큰 버퍼에서 사용할 부분만큼만 연다

private:
	SendBufferChunkRef	Pop();
	void				Push(SendBufferChunkRef buffer);

	static void			PushGlobal(SendBufferChunk* buffer);


private:
	USE_LOCK;
	Vector<SendBufferChunkRef> _sendBufferChunks;

};
