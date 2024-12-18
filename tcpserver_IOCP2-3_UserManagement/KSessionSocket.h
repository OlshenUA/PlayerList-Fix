#pragma once
#include "KSocket.h"
#include <memory>

class KSession;
class KSessionSocket;
typedef std::shared_ptr<KSessionSocket> KSessionSocketPtr;
typedef std::weak_ptr<KSessionSocket> KSessionSocketWeakPtr;
class KSessionSocket : public KSocket
{
public:
    KSessionSocket();
    virtual ~KSessionSocket();
    virtual void OnReceiveCompleted(DWORD dwTransferred_);
    virtual void OnCloseSocket();
    virtual void OnAcceptConnection();

    KSession* m_pkSession; // qff. must be std::weak_ptr<KSession>
};
