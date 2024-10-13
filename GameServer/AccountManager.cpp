#include "pch.h"
#include "AccountManager.h"
#include "UserManager.h"

void AccountManager::ProcessLogin()
{
	// accountLock
	lock_guard<mutex> guard(_mutex);

	// userLock -> 내부적으로 락을 호출해서 두 개의 락을 잡게 됨
	User* user = UserManager::Instance()->GetUser(100);

	// TODO
}