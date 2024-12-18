#pragma once
#define CEREAL_THREAD_SAFE 1
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

enum EPacketId
{
	EPACKET_ACCEPT_CONNECTION = 0,
	EPACKET_CREATE_USER, // server to client
	EPACKET_DESTROY_USER, // server to client
	EPACKET_CHAT, // client to server, server to all client
	EPACKET_USER_INFO, // client to server, server to all client
	EPACKET_USER_LIST // server to client
};

struct KPacketAcceptConnection
{
	unsigned long _dwKey;
};

template <typename Archive>
void serialize(Archive& ar, KPacketAcceptConnection& a, const unsigned int version)
{
	ar& a._dwKey;
}//serialize()

struct KPacketCreateUser
{
	unsigned long _dwKey;
	std::string _name;
};

template <typename Archive>
void serialize(Archive& ar, KPacketCreateUser& a, const unsigned int version)
{
	ar& a._dwKey;
	ar& a._name;
}//serialize()

typedef struct KPacketCreateUser	KPacketDestroyUser;


struct KPacketChat
{
	std::string _text;
};

template <typename Archive>
void serialize(Archive& ar, KPacketChat& a, const unsigned int version)
{
	ar& a._text;
}//serialize()

struct KPacketUserInfo
{
	unsigned int _dwUserKey;
	int	_xPos;
	int	_yPos;
};

template <typename Archive>
void serialize(Archive& ar, KPacketUserInfo& a, const unsigned int version)
{
	ar& a._dwUserKey;
	ar& a._xPos;
	ar& a._yPos;
}//serialize()

struct KPacketUserList
{
	std::vector<KPacketUserInfo>	_vecUsers;
};

template <typename Archive>
void serialize(Archive& ar, KPacketUserList& a, const unsigned int version)
{
	ar& a._vecUsers;
}//serialize()

#pragma pack( push, 1 )
class KPacket;
typedef std::shared_ptr<KPacket>	KPacketPtr;
typedef std::weak_ptr<KPacket>		KPacketWeakPtr;
class KPacket
{
public:
	template <class T>
	void SetData(unsigned int nSenderUID, unsigned short usPacketId, const T& data);

	unsigned int        m_nSenderUid;
	unsigned short      m_usPacketId;
	std::vector<char>   m_buffer;
};//class KPacket
#pragma pack( pop )

template <class T>
void KPacket::SetData(unsigned int nSenderUID, unsigned short usPacketId, const T& data_)
{
	m_nSenderUid = nSenderUID;
	m_usPacketId = usPacketId;

	std::stringstream   ss;
	cereal::BinaryOutputArchive oa(ss); // Create an output archive
	oa(data_);

	std::string str = ss.str();
	m_buffer.reserve(str.size());
	m_buffer.assign(str.begin(), str.end());
}//KPacket::SetData()


template <typename Archive>
void serialize(Archive& ar, KPacket& a, const unsigned int version)
{
	ar& a.m_nSenderUid;
	ar& a.m_usPacketId;
	ar& a.m_buffer;
}//serialize()

template <typename T>
void BufferToPacket(IN std::stringstream& ss_, OUT T& packet_)
{
	cereal::BinaryInputArchive ia(ss_); // Create an input archive
	ia(packet_);
}//BufferToPacket()

template <typename T>
void BufferToPacket(IN std::vector<char>& buffer, OUT T& data)
{
	std::stringstream ss;
	std::copy(buffer.begin(), buffer.end(), std::ostream_iterator<char>(ss));
	cereal::BinaryInputArchive ia(ss); // Create an input archive
	ia(data);
}//BufferToPacket()

template<typename T>
void PacketToBuffer(IN T& packet_, OUT std::stringstream& ss_)
{
	cereal::BinaryOutputArchive oa(ss_); // Create an output archive
	oa(packet_);
}//PacketToBuffer()

template<typename T>
void PacketToBuffer(IN T& packet_, OUT std::vector<char>& buffer_)
{
	std::stringstream   ss;
	cereal::BinaryOutputArchive oa(ss); // Create an output archive
	oa(packet_);

	// set [out] parameter
	std::string str = ss.str();
	buffer_.reserve(str.size());
	buffer_.assign(str.begin(), str.end());
}//PacketToBuffer()
