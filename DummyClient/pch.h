#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib") // 라이브러리를 긁어옴
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib") // 라이브러리를 긁어옴
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib") // 라이브러리를 긁어옴
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib") // 라이브러리를 긁어옴
#endif

#include "CorePch.h"

