#include "KGen.h"
#include "KThreadManager.h"
#include <functional>
#include <algorithm>

using namespace std::placeholders;


CONSTRUCTOR KThreadManager::KThreadManager() 
{
}

DESTRUCTOR KThreadManager::~KThreadManager()
{
}

VIRTUAL bool KThreadManager::Initialize( DWORD nThreadNum_ )
{
    //// create I/O threads

    std::lock_guard<std::mutex> lock(m_muVecThread);

    for( UINT i = 0; i < nThreadNum_; ++i ) {
        std::shared_ptr<KThread> spThread( CreateThread() );
        spThread->SetThreadManager( this );
        m_vecThread.push_back( spThread );
    }
    return true;
}

VIRTUAL void KThreadManager::Finalize()
{
}

void KThreadManager::BeginAllThread()
{
    std::lock_guard<std::mutex> lock(m_muVecThread);

    std::for_each( m_vecThread.begin(), m_vecThread.end(), std::bind( &KThread::BeginThread, _1 ) );
}

void KThreadManager::EndAllThread()
{
    std::lock_guard<std::mutex> lock(m_muVecThread);

    if( m_vecThread.empty() )
        return;

    std::for_each( m_vecThread.begin(), m_vecThread.end()
        , std::bind( &KThread::EndThread, _1, 3000 ) );

    BEGIN_LOG( cout, L"waiting termination of all threads" );

    m_vecThread.clear();
}

void KThreadManager::SetNumThread( size_t nThreadNum_ )
{
    std::lock_guard<std::mutex> lock(m_muVecThread);

    if( nThreadNum_ == m_vecThread.size() ) {
        BEGIN_LOG( cout, L"Num threads : " )
            << LOG_NAMEVALUE( nThreadNum_ );
        return;
    }

    BEGIN_LOG( cout, L"thread count : " )
        << LOG_NAMEVALUE( m_vecThread.size() )
        << LOG_NAMEVALUE( nThreadNum_ );

    if( nThreadNum_ < m_vecThread.size() ) {
        m_vecThread.resize( nThreadNum_ );
    }
    else { // increase # of threads
        for( size_t i = m_vecThread.size(); i < nThreadNum_; ++i ) {
            std::shared_ptr<KThread> spThread( CreateThread() );
            spThread->SetThreadManager( this );
            spThread->BeginThread();
            m_vecThread.push_back( spThread );
        }
    }

    BEGIN_LOG( cout, L"final thread count : " )
        << LOG_NAMEVALUE( m_vecThread.size() );
}
