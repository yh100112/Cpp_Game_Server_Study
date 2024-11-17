#pragma once

/*-----------------
	SendBuffer
-----------------*/
class SendBuffer : enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(int32 bufferSize);
	~SendBuffer();

	BYTE* Buffer() { return _buffer.data(); }
	int32 WriteSize() { return _writeSize; }
	int32 Capacity() { return static_cast<int32>(_buffer.size()); } 

	void CopyData(void* data, int32 len); // 버퍼에 데이터를 밀어넣는 작업

private:
	Vector<BYTE>	_buffer;		// 전체 버퍼
	int32			_writeSize = 0; // 실제 write에 사용된 버퍼 사이즈
};

