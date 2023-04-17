#include "NetDataManager.h"
#include <QDataStream>
#include "netPack.h"
#include "netPackDefine.h"
#include "LogManager.h"

//NetDataManager
NetDataManager::NetDataManager()
{    
    mRecvSize = 1024 * 1024;
    mRecvBuffer = new char[mRecvSize];

	packHead.resize(8);
	packHead.fill((char)0xff);
		
	packTail.append(0x5A);	
}

NetDataManager::~NetDataManager() 
{
	delete []mRecvBuffer;
}

void NetDataManager::newSession(int sessionId, QTcpSocket *socket)
{	
	m_sessionInfo[sessionId].socket = socket;	
}

void NetDataManager::endSession(int sessionId)
{	
	m_sessionInfo.remove(sessionId);	
}

void NetDataManager::recvPack(int sessionId)
{	
	QTcpSocket *socket = m_sessionInfo[sessionId].socket;
	while(true)
	{
        int read_len = socket->read(mRecvBuffer,mRecvSize);		
		if (read_len > 0)
		{
			m_sessionInfo[sessionId].buffer.append(mRecvBuffer, read_len);			
		}
        else
        {
            if(read_len == -1)
				m_sessionInfo[sessionId].buffer.clear();

			break;
        }
	}

	parsePack(sessionId);
}

void NetDataManager::recvData(int sessionId, QByteArray data)
{
	m_sessionInfo[sessionId].buffer.append(data);
	parsePack(sessionId);
}

QByteArray NetDataManager::packData(const NetPack &pack)
{	
	QByteArray send_buffer;	
	QDataStream s(&send_buffer,QDataStream::WriteOnly);	

	pack.pack(data_buffer);		
	pack.info.Pack(head_buffer);	
	
    send_buffer.append(packHead);	
	send_buffer.append(head_buffer);
	send_buffer.append(data_buffer);	

	uchar chkSum = CaculateSum((uchar*)send_buffer.data() + 8,send_buffer.length() - 8);
	send_buffer.append(chkSum);
    send_buffer.append(packTail);

	return send_buffer;
}

bool NetDataManager::sendPack(int sessionId, const NetPack &pack,bool sync)
{		
	QByteArray send_buffer = packData(pack);

	QTcpSocket *socket = m_sessionInfo[sessionId].socket;
	Q_ASSERT(socket);
    
    bool ret = true;

    const char *ptr = send_buffer.data();
    int size = send_buffer.size();    
    socket->write(ptr,size);        		
	return ret;
}

NetPackPtr NetDataManager::takePack(int sessionId,int type,int param)
{	
	NetPackPtr pack;
	QVector<NetPackPtr> &pack_list = m_sessionInfo[sessionId].packList;
	if (pack_list.size() > 0)
	{
        if(type == 0) //取第一个
        {
		    pack = pack_list[0];
		    pack_list.pop_front();
        }
        else
        {
            for(int i = 0; i < pack_list.size(); i++)
            {
                if((type == 1 && pack_list[i]->type() == param))
                {
                    pack = pack_list[i];
                    pack_list.remove(i);
                    break;
                }
            }
        }
	}
	
	return pack;
}

NetPackPtr NetDataManager::takePack(int sessionId)
{
	return takePack(sessionId, 0, 0);
}

NetPackPtr NetDataManager::takePackByType(int sessionId, int type)
{
	return takePack(sessionId, 1, type);
}

NetPackPtr NetDataManager::takePackBySeq(int sessionId, int seq)
{
	return takePack(sessionId, 2, seq);
}

void NetDataManager::parsePack(int sessionId)
{
	QByteArray &buffer = m_sessionInfo[sessionId].buffer;
	PackInfo head;	
	QByteArray pack_buffer;
	QByteArray tail_buffer;	
		
	int CHECK_SIZE = 1;

	int head_size = 8;
    int min_pack_head = 6;
	int tail_size = 2;  //tail + chk

	int min_pack_len = packHead.length() + min_pack_head + packTail.length();
	int buffer_start = 0;
	while (buffer.length() - buffer_start >= min_pack_len)
	{
		int start = buffer.indexOf(packHead, buffer_start);
		int end = 0;
		int headOffset = 0;
		if (start < 0)
		{			
			buffer_start = buffer.length(); //全是无用数据，清除
			break;
		}

		//接收数据不全
		start += packHead.length();	
		headOffset = start;
		if (!head.UnPack(buffer, start))
			return;

		int pack_info_len = head.IsRecvPack() ? 7 : 6;
		start += pack_info_len;
		
		int bodySize = head.Length - 8 - 2 - pack_info_len;
		if (bodySize < 0)
		{			
			buffer_start = buffer.length(); //全是无用数据，清除
			break;
		}

		//接收数据不全
		if (start + bodySize + packTail.length() > buffer.length())
		{
			Q_ASSERT(0); //upack 的时候已经检查了
			break;
		}

		end = start + bodySize;
		uchar chk = buffer[end]; 				
		tail_buffer = buffer.mid(end, packTail.length());
		if (tail_buffer != packTail)
		{			
			buffer_start = buffer.length(); //全是无用数据，清除
			break;
		}
		pack_buffer = buffer.mid(start, bodySize);

		NetPack *pack = NetPackManager::instance()->createPack(NETPACK_VARIANT);
		pack->info = head;
		if (pack->unPack(pack_buffer))
			m_sessionInfo[sessionId].packList.push_back(NetPackPtr(pack));
		else
			delete pack;
		start += bodySize + CHECK_SIZE + packTail.length();

		//下一次读起始
		buffer_start = start;
	}
	buffer = buffer.mid(buffer_start);
}