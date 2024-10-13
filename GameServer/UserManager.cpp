#include "pch.h"
#include "UserManager.h"
#include "AccountManager.h"

void UserManager::ProcessSave()
{
	// userLock
	lock_guard<mutex> guard(_mutex);

	// accountLock
	Account* account = AccountManager::Instance()->GetAccount(100);

	// 이렇게 순서를 바꾸면 데드락 해결됨
	//// accountLock
	//Account* account = AccountManager::Instance()->GetAccount(100);

	//// userLock
	//lock_guard<mutex> guard(_mutex);


	// TODO
}