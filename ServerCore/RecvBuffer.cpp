#include "pch.h"
#include "RecvBuffer.h"

/*------------
	RecvBuffer
-------------*/

RecvBuffer::RecvBuffer(int32 bufferSize) : _bufferSize(bufferSize)
{
	_capacity = bufferSize * BUFFER_COUNT;
	_buffer.resize(_capacity); // ���� ����� ���ۺ��� 10��� ����
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::Clean()
{
	int32 dataSize = DataSize();
	if (dataSize == 0) // �� r, w Ŀ���� ������ ��ġ��� �� �� �� ������ ����
	{
		_readPos = _writePos = 0;
	}
	else // r, w�� �� ������ �̵�
	{
		// ���� ������ ���� ���� 1�� ũ�� �̸��̸�, �����͸� ������ �����.
		// [][][][][] [][][][][] [][][][][r] [w][][][][]
		if (FreeSize() < _bufferSize)
		{
			::memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
			_readPos = 0;
			_writePos = dataSize;
		}
	}
}

// ���������� numOfBytes��ŭ read ������ r Ŀ���� ������ �̵���Ű�� ����
bool RecvBuffer::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize())
		return false;

	_readPos += numOfBytes;
	return true;
}

// ���������� numOfBytes��ŭ write ������ w Ŀ���� ������ �̵���Ű�� ����
bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	if (numOfBytes > FreeSize())
		return false;

	_writePos += numOfBytes;
	return true;
}
