#include "KSession.h"

#define DEFAULT_BUFFER      2048

using namespace std::placeholders;

KSession::KSession()
{
    m_socket.m_pkSession = this;
}

VIRTUAL KSession::~KSession()
{
}

bool KSession::SendPacket(KPacketPtr spPacket)
{
    std::vector<char>   buffer;
    PacketToBuffer(IN * spPacket, OUT buffer);
    bool ret = m_socket.SendData((const char*)&buffer[0], (int)buffer.size());
    return ret;
}

void KSession::AddPacket(KPacketPtr spPacket)
{
    m_kQueue.AddPacket(spPacket);
}

VIRTUAL void KSession::Update(float fElapsedTime)
{
    m_kQueue.ProcessAllPacket(std::bind(&KSession::OnPacket, this, _1));
}

VIRTUAL void KSession::OnPacket(KPacketPtr spPacket)
{
    printf("key = %u, %s id=%i\r\n"
        , m_socket.GetKey(), __FUNCTION__, spPacket->m_usPacketId); // qff
}

VIRTUAL void KSession::OnAcceptConnection()
{
    //KPacket packet;
    //packet.SetData(EGSCL_ACCEPT_CONNECTION_NOT, KIdPacket());
    //SendPacket(packet);
    KPacketAcceptConnection connection;
    connection._dwKey = m_socket.GetKey();
    SendPacket(EPACKET_ACCEPT_CONNECTION, connection);
    BEGIN_LOG(cout, L"key: " << connection._dwKey)
        << END_LOG;
}

void KSession::OnReceiveData(IN std::vector<char>& buffer_)
{
    KPacketPtr spPacket(new KPacket());

    BufferToPacket(IN buffer_, IN OUT * spPacket); // qff
    //spPacket->SetData(0, 0, buffer_);
    AddPacket(spPacket);
}

VIRTUAL void KSession::OnCloseSocket()
{
    //m_bDestroyReserved = true;
}
