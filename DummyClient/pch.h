#pragma once

#define WIN32_LEAN_AND_MEAN // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib") // ���̺귯���� �ܾ��
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib") // ���̺귯���� �ܾ��
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib") // ���̺귯���� �ܾ��
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib") // ���̺귯���� �ܾ��
#endif

#include "CorePch.h"

