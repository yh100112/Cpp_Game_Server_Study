#pragma once

/*------------
	RecvBuffer
-------------*/
// recv buffer ������ ���� ���
// ���۸� ������ �ΰ� ����
// r, w  Ŀ���� ���� ��ġ�� �ȴٸ� �� �̻� ó���� �����Ͱ� ���ٴ� ���̹Ƿ�
// r, w�� �� �� 0�� ��ġ�� �̵�������. �̷��� �Ǹ� ���� ����� ����
// ���� w�� �� ���� �����ϸ� r���� w������ �� ������ ���������
// [r][][][][w]  [][][][][]  [][][][][]  [][][][][]  [][][][][]
class RecvBuffer
{
	enum { BUFFER_COUNT = 10 };
public:
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();

	void				Clean();
	bool				OnRead(int32 numOfBytes);
	bool				OnWrite(int32 numOfBytes);

	BYTE* ReadPos()		{ return &_buffer[_readPos]; }
	BYTE* WritePos()	{ return &_buffer[_writePos]; }
	int32 DataSize()	{ return _writePos - _readPos; }
	int32 FreeSize()	{ return _capacity - _writePos; } // ����� �� �ִ� ������ ���ΰ�?


private:
	int32				_capacity = 0;
	int32				_bufferSize = 0; // ���� ������ ������
	// [r][][][][w]  [][][][][]    [][][][][]    [][][][][]    [][][][][]
	// <bufferSize>  <bufferSize>  <bufferSize>
	
	int32				_readPos = 0;  // Ŀ����� ����
	int32				_writePos = 0; // Ŀ����� ����
	Vector<BYTE>		_buffer;
};

