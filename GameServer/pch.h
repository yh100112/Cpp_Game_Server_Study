/*
GameServer ������Ʈ���� ���� ���Ǵ� ������� pch�� �־������ ��׵��� ���尡 �ѹ��� 
�Ǿ �̸� ���� �� ������
������ �ֵ��� ��������� Ȱ���� �� ���� ������� �ʰ� ������ ����� �� �ִٴ� ������ �ִ�.
�������δ� �� ���� �����ϵ� ����� �����ϸ� ���� �κп� ������ �ش�.
*/

#pragma once

#define WIN32_LEAN_AND_MEAN // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib") // ���̺귯���� �ܾ��
#else
#pragma comment(lib, "Release\\ServerCore.lib") // ���̺귯���� �ܾ��
#endif

#include "CorePch.h"