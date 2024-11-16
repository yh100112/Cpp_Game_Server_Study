#pragma once

/*------------
	RecvBuffer
-------------*/
// recv buffer 구현할 동작 방식
// 버퍼를 여유를 두고 만듬
// r, w  커서가 같은 위치가 된다면 더 이상 처리할 데이터가 없다는 뜻이므로
// r, w를 둘 다 0번 위치로 이동시켜줌. 이렇게 되면 복사 비용이 없음
// 만약 w만 맨 끝에 도달하면 r부터 w까지만 맨 앞으로 복사시켜줌
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
	int32 FreeSize()	{ return _capacity - _writePos; } // 사용할 수 있는 공간이 얼마인가?


private:
	int32				_capacity = 0;
	int32				_bufferSize = 0; // 단일 버퍼의 사이즈
	// [r][][][][w]  [][][][][]    [][][][][]    [][][][][]    [][][][][]
	// <bufferSize>  <bufferSize>  <bufferSize>
	
	int32				_readPos = 0;  // 커서라고 생각
	int32				_writePos = 0; // 커서라고 생각
	Vector<BYTE>		_buffer;
};

