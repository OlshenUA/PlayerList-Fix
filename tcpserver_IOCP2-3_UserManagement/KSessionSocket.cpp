#include "KSessionSocket.h"
#include "KSession.h"


KSessionSocket::KSessionSocket()
{
}

VIRTUAL KSessionSocket::~KSessionSocket()
{
}

VIRTUAL void KSessionSocket::OnReceiveCompleted(DWORD dwTransferred_)
{
    if (dwTransferred_ == 0) {
        BEGIN_LOG(cout, L"closed socket: ")
            << LOG_NAMEVALUE(dwTransferred_)
            << END_LOG;
        VIRTUAL OnCloseSocket();
        return;
    }

    DWORD usPacketLen = 0;

    m_ovlReceive.m_dwRemained += dwTransferred_;

    usPacketLen = dwTransferred_;
    while (m_ovlReceive.m_dwRemained >= 1) {
        // get packet length
        //memcpy(OUT & usPacketLen, m_ovlReceive.m_pBuffer, sizeof(usPacketLen));

        // if we can build complete packet, then process it
        if (m_ovlReceive.m_dwRemained >= usPacketLen) {
            std::vector<char>   buffer;
            buffer.insert(buffer.end(), m_ovlReceive.m_pBuffer
                , m_ovlReceive.m_pBuffer + usPacketLen);

            if (m_pkSession != nullptr)
                m_pkSession->OnReceiveData(buffer);

            // buffer management
            m_ovlReceive.m_dwRemained -= usPacketLen;
            memmove(&m_ovlReceive.m_pBuffer[0], &m_ovlReceive.m_pBuffer[usPacketLen], m_ovlReceive.m_dwRemained);
            usPacketLen = m_ovlReceive.m_dwRemained;// qff. 20241125_jintaeks
        }
        else {
            break; // incomplete data to build a packet.
        }//if.. else..
    }

    // start next receiving.
    KSocket::ReceiveData();
}

VIRTUAL void KSessionSocket::OnCloseSocket()
{
    if (m_pkSession != nullptr)
        VIRTUAL m_pkSession->OnCloseSocket();
}

VIRTUAL void KSessionSocket::OnAcceptConnection()
{
    if (m_pkSession != nullptr)
        VIRTUAL m_pkSession->OnAcceptConnection();
}
