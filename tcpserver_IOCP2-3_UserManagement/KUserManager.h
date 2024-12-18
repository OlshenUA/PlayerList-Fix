#pragma once
#include <map>
#include "KUser.h"
#include <mutex>

class KUserManager;
typedef std::shared_ptr<KUserManager> KUserManagerPtr;
class KUserManager
{
public:
    void CreateUser(KUserPtr spUser);
    void Update(float fElapsedTime);
    template<typename T>
    void SendToAll(unsigned int packetId, const T& data);
    void OnCommand(KUserPtr sp, KUser::EUserCommand command);
    KUserPtr GetUser(const DWORD nUid_);
    bool DeleteUser(const DWORD dwUserKey);
    template<typename T>
    bool SendTo(DWORD dwUserKey, unsigned int packetId, const T& data);

private:
    std::recursive_mutex _mutex;
    std::map<DWORD, KUserPtr>   _users;
};


template<typename T>
void KUserManager::SendToAll(unsigned int packetId, const T& data_)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);

    std::map<DWORD, KUserPtr>::iterator it = _users.begin();
    while (it != _users.end()) {
        DWORD dwKey = it->first;
        KUserPtr sp = it->second;
        if (sp) {
            sp->SendPacket(packetId, data_);
        }
        ++it;
    }
}

template<typename T>
bool KUserManager::SendTo(DWORD dwUserKey, unsigned int packetId, const T& data_)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);

    KUserPtr sp = GetUser(dwUserKey);
    if (sp) {
        sp->SendPacket(packetId, data_);
        return true;
    }

    return false;
}
