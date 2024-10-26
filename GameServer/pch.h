/*
GameServer 프로젝트에서 자주 사용되는 헤더들을 pch에 넣어놓으면 얘네들이 빌드가 한번에 
되어서 미리 빌드 된 다음에
나머지 애들이 이헤더들을 활용할 때 재차 빌드되지 않고 빠르게 사용할 수 있다는 장점이 있다.
단점으로는 이 프리 컴파일드 헤더를 수정하면 많은 부분에 영향을 준다.
*/

#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib") // 라이브러리를 긁어옴
#else
#pragma comment(lib, "Release\\ServerCore.lib") // 라이브러리를 긁어옴
#endif

#include "CorePch.h"