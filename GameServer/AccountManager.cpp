#include "pch.h"
#include "AccountManager.h"
#include "UserManager.h"

void AccountManager::ProcessLogin()
{
	// accountLock
	lock_guard<mutex> guard(_mutex);

	// userLock -> ���������� ���� ȣ���ؼ� �� ���� ���� ��� ��
	User* user = UserManager::Instance()->GetUser(100);

	// TODO
}