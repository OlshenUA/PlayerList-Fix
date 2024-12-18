#include "KUser.h"
#include <string.h>

KUser::KUser()
{
    _eUserState = EUSER_STATE_UNINITIALIZED;
    _dwKey = 0L;
    _isDestroyPending = false;
}

VIRTUAL KUser::~KUser()
{
}

void KUser::SendText(const char* text)
{
    int len = strlen(text);
    m_socket.SendData(text, len);
}

void KUser::SetUserState(EUserState userState)
{
    _eUserState = userState;
}

VIRTUAL void KUser::Update(float fElapsedTime)
{
    __super::Update(fElapsedTime);
    if (_eUserState == EUSER_STATE_CREATED) {
        DWORD dwKey = m_socket.GetKey();
        if (dwKey != _dwKey) {
            _dwKey = dwKey;
            SetUserState(EUSER_STATE_IOCP_BOUNDED);
            AddCommand(EUSER_COMMAND_GETPLAYERLIST);
            AddCommand(EUSER_COMMAND_CREATEUSER);
        }
    }
    else if (_eUserState == EUSER_STATE_IOCP_BOUNDED) {
    }
}

void KUser::AddCommand(EUserCommand command)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _commandQueue.push(command);
}

bool KUser::IsPendingCommand()
{
    int size;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        size = _commandQueue.size();
    }
    return size;
}

void KUser::ClearPendingCommand()
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::queue<EUserCommand> emptyQueue;
    _commandQueue.swap(emptyQueue);
}

VIRTUAL void KUser::OnAcceptConnection()
{
    __super::OnAcceptConnection();
}

VIRTUAL void KUser::OnCloseSocket()
{
    __super::OnCloseSocket();
    //AddCommand("DestroyUser");
    _isDestroyPending = true;
}

VIRTUAL void KUser::OnPacket(KPacketPtr spPacket)
{
    __super::OnPacket(spPacket);

    DWORD dwPacketId = spPacket->m_usPacketId;
    if (dwPacketId == EPACKET_CHAT) {
        KPacketChat chat;
        BufferToPacket(IN spPacket->m_buffer, OUT chat);
        //AddCommand(EUSER_COMMAND_CHAT);
    }
    else if (dwPacketId == EPACKET_USER_INFO) {
        KPacketUserInfo userInfo;
        BufferToPacket(IN spPacket->m_buffer, OUT userInfo);
        SetXYPos(userInfo._xPos, userInfo._yPos);
        AddCommand(EUSER_COMMAND_UPDATEUSERINFO);
    }
}
