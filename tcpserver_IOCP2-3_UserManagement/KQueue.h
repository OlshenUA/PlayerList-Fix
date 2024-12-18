#pragma once
#include <thread>
#include <mutex>
#include "KPacket.h"
#include <queue>

class KQueue;
typedef std::shared_ptr<KQueue>	    KQueuePtr;
typedef std::weak_ptr<KQueue>		KQueueWeakPtr;
class KQueue
{
public:
    void AddPacket(KPacketPtr spPacket)
    {
        std::lock_guard<std::mutex> lock(m_muQueue);
        m_queue.push(spPacket);
    }
    template<typename FUNCTOR>
    void ProcessAllPacket(FUNCTOR callback)
    {
        while (true) {
            KPacketPtr spPacket;
            {
                std::lock_guard<std::mutex> lock(m_muQueue);
                if (m_queue.empty())
                    break;
                spPacket = m_queue.front();
                m_queue.pop();
            }
            callback(spPacket);
        }
    }
private:
    std::queue<KPacketPtr>  m_queue;
    std::mutex              m_muQueue;
};
