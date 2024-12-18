#pragma once
#include "KGen.h"
#include "KSession.h"
#include <mutex>

class KUser;
typedef std::shared_ptr<KUser> KUserPtr;
class KUser : public KSession
{
public:
    enum EUserState
    {
        EUSER_STATE_UNINITIALIZED = 0,
        EUSER_STATE_CREATED,
        EUSER_STATE_IOCP_BOUNDED,
    };

    enum EUserCommand
    {
        EUSER_COMMAND_GETPLAYERLIST=0,
        EUSER_COMMAND_CREATEUSER,
        EUSER_COMMAND_CHAT,
        EUSER_COMMAND_UPDATEUSERINFO,
    };

public:
    KUser();
    virtual ~KUser();
    void SendText(const char* text);
    void SetUserState(EUserState userState);
    EUserState GetUserState() const { return _eUserState; }
    virtual void Update(float fElapsedTime) override;
    void AddCommand(EUserCommand command);
    bool IsPendingCommand(); // must be replaced with command queue
    void ClearPendingCommand(); // must be replaced with command queue
    virtual void OnAcceptConnection() override;
    bool IsDestroyPending() { return _isDestroyPending; }

    template<typename FUNCTOR>
    void ProcessAllCommand(FUNCTOR callback)
    {
        while (true) {
            EUserCommand command;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if (_commandQueue.empty())
                    break;
                command = _commandQueue.front();
                _commandQueue.pop();
            }
            callback(command);
        }
    }

    virtual void OnPacket(KPacketPtr spPacket) override;

    DWORD GetSocketKey() { return _dwKey; }
    DWORD GetUserKey() { return _dwUserKey; }
    void SetUserKey(DWORD dwUserKey) { _dwUserKey = dwUserKey; }
    void GetXYPos(int& xOut, int& yOut) { xOut = _xPos; yOut = _yPos; }
    void SetXYPos(int x, int y) { _xPos = x; _yPos = y; }

protected:
    virtual void OnCloseSocket() override;

private:
    EUserState      _eUserState;
    DWORD           _dwKey; // socket key
    DWORD           _dwUserKey; // user key(key to user manager)
    std::mutex      _mutex;
    bool _isDestroyPending;
    std::queue<EUserCommand> _commandQueue;
    int             _xPos;
    int             _yPos;
};
