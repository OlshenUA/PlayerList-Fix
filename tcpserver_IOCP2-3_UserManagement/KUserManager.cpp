#include "KUserManager.h"

using namespace std::placeholders;


void KUserManager::CreateUser(KUserPtr spUser)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    spUser->SetUserState(KUser::EUSER_STATE_CREATED);
    const DWORD dwKey = reinterpret_cast<DWORD>(spUser.get());
    spUser->SetUserKey(dwKey);

    // qff
    //KPacketCreateUser createUser;
    //createUser._dwKey = dwKey;
    //createUser._name = "";
    //SendToAll(EPACKET_CREATE_USER, createUser);

    _users.insert(std::make_pair(dwKey, spUser));
    BEGIN_LOG(cout, L"CreateUser(): number of users = " << _users.size())
        << END_LOG;
}

void KUserManager::Update(float fElapsedTime)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    std::vector<std::pair<DWORD,DWORD>> keysToDelete;
    std::map<DWORD, KUserPtr>::iterator it = _users.begin();
    while (it != _users.end()) {
        DWORD key = it->first;
        KUserPtr sp = it->second;
        if (sp) {
            sp->Update(fElapsedTime);
            if (sp->IsPendingCommand()) {
                sp->ProcessAllCommand(std::bind(&KUserManager::OnCommand, this, sp, _1));
            }
            if (sp->IsDestroyPending())
                keysToDelete.push_back(std::pair<DWORD,DWORD>(key,sp->GetSocketKey()));
        }
        ++it;
    }
    for (std::pair<DWORD,DWORD> key : keysToDelete) {
        //_users.erase(key.first);
        DeleteUser(key.first);
        // send destroy user packet to all clients. qff
        KPacketDestroyUser destroy;
        destroy._dwKey = key.second;
        SendToAll(EPACKET_DESTROY_USER, destroy);
    }
}

void KUserManager::OnCommand(KUserPtr sp, KUser::EUserCommand command)
{
    if (sp == nullptr)
        return;

    if (command == KUser::EUSER_COMMAND_CREATEUSER) {
        KPacketCreateUser createUser;
        createUser._dwKey = sp->GetSocketKey();
        createUser._name = "";
        SendToAll(EPACKET_CREATE_USER, createUser);
    }
    else if (command == KUser::EUSER_COMMAND_UPDATEUSERINFO) {
        KPacketUserInfo userInfo;
        userInfo._dwUserKey = sp->GetSocketKey();
        sp->GetXYPos( OUT userInfo._xPos, OUT userInfo._yPos);
        SendToAll(EPACKET_USER_INFO, userInfo);
    }
    else if (command == KUser::EUSER_COMMAND_GETPLAYERLIST) {
        KPacketUserList userList;

        for (auto& pair : _users) {
            KUserPtr user = pair.second;
            if (user) {
                KPacketUserInfo userInfo;
                userInfo._dwUserKey = user->GetSocketKey();
                user->GetXYPos(OUT userInfo._xPos, OUT userInfo._yPos);
                userList._vecUsers.push_back(userInfo);
            }
        }
        SendToAll(EPACKET_USER_LIST, userList);
    }
}

KUserPtr KUserManager::GetUser(const DWORD dwUserKey)
{
    auto mit = _users.find(dwUserKey);

    if (mit == _users.end())
        return KUserPtr();

    return mit->second;
}

VIRTUAL bool KUserManager::DeleteUser(const DWORD dwUserKey)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);

    auto mit = _users.find(dwUserKey);
    if (mit == _users.end()) {
        return false;
    }

    _users.erase(dwUserKey);
    BEGIN_LOG(cout, L"Delete User(): number of users = " << _users.size())
        << END_LOG;
    return true;
}
