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
	BYTE*					_buffer;		// sendbufferchunk�� ����ϹǷ� �����͸� ��������� ��
	uint32					_allocSize = 0;
	uint32					_writeSize = 0; // ���� write�� ���� ���� ������
	SendBufferChunkRef		_owner;			// ���� �����ϰ� ������ �ʴ� ���� ������� �ȵ�!
};


/*-----------------
	SendBufferChunk ( ������ ū ���� �Ѱ��� �����ϰ� �ʿ��� ������ �ɰ��� ����ϴ� ��å )
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
	SendBufferRef		Open(uint32 allocSize); // ������ ���� ( �Ҵ��� ���� )
	void				Close(uint32 writeSize); // ���������� ����� ���� = writeSize

	bool				IsOpen() { return _open; }
	BYTE*				Buffer() { return &_buffer[_usedSize]; } // ����� �� ���� ��ġ�� �ּҸ� ��ȯ
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
	SendBufferRef Open(uint32 size); // ������ ū ���ۿ��� ����� �κи�ŭ�� ����

private:
	SendBufferChunkRef	Pop();
	void				Push(SendBufferChunkRef buffer);

	static void			PushGlobal(SendBufferChunk* buffer);


private:
	USE_LOCK;
	Vector<SendBufferChunkRef> _sendBufferChunks;

};
