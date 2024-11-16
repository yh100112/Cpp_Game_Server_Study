#include "pch.h"
#include "RecvBuffer.h"

/*------------
	RecvBuffer
-------------*/

RecvBuffer::RecvBuffer(int32 bufferSize) : _bufferSize(bufferSize)
{
	_capacity = bufferSize * BUFFER_COUNT;
	_buffer.resize(_capacity); // 실제 사용할 버퍼보다 10배로 잡음
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::Clean()
{
	int32 dataSize = DataSize();
	if (dataSize == 0) // 딱 r, w 커서가 동일한 위치라면 둘 다 맨 앞으로 리셋
	{
		_readPos = _writePos = 0;
	}
	else // r, w를 맨 앞으로 이동
	{
		// 여유 공간이 단일 버퍼 1개 크기 미만이면, 데이터를 앞으로 땅긴다.
		// [][][][][] [][][][][] [][][][][r] [w][][][][]
		if (FreeSize() < _bufferSize)
		{
			::memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
			_readPos = 0;
			_writePos = dataSize;
		}
	}
}

// 성공적으로 numOfBytes만큼 read 했으면 r 커서를 앞으로 이동시키는 역할
bool RecvBuffer::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize())
		return false;

	_readPos += numOfBytes;
	return true;
}

// 성공적으로 numOfBytes만큼 write 했으면 w 커서를 앞으로 이동시키는 역할
bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	if (numOfBytes > FreeSize())
		return false;

	_writePos += numOfBytes;
	return true;
}
