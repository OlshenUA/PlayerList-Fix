#pragma once
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <memory>
#include "KQueue.h"
#include "KPacket.h"
#include "KSessionSocket.h"

class KSession;
typedef std::shared_ptr<KSession>  KSessionPtr;
typedef std::weak_ptr<KSession>  KSessionWeakPtr;
class KSession: public noncopyable
{
    friend class KSessionSocket;
    friend class KIocpWorkerThread;

public:
    KSession();
    virtual ~KSession();
    bool SendPacket(KPacketPtr spPacket);
    template<typename T>
    bool SendPacket(unsigned short usPacketId_, T& data_);
    void AddPacket(KPacketPtr spPacket);
    virtual void Update(float fElapsedTime);
    KSocket* GetKSocket() { return &m_socket; }
    virtual void OnPacket(KPacketPtr spPacket);

    virtual void OnAcceptConnection();

protected:
    void OnReceiveData(IN std::vector<char>& buffer_);
    virtual void OnCloseSocket();

protected:
    KQueue          m_kQueue;
    KSessionSocket  m_socket;
};

template<typename T>
bool KSession::SendPacket(unsigned short usPacketId_, T& data_)
{
    KPacketPtr spPacket;
    spPacket.reset( new KPacket());
    spPacket->SetData(m_socket.GetKey(), usPacketId_, data_);

    return KSession::SendPacket(spPacket);
}
