#include "JZNetDataManager.h"
#include <QDataStream>

//JZNetDataManager
JZNetDataManager::JZNetDataManager()
{    
    mRecvSize = 1024 * 1024;
    mRecvBuffer = new char[mRecvSize];
	packHead = (char)0xAA;		
	packTail = 0x55;	
    m_packSeq = 0;
}

JZNetDataManager::~JZNetDataManager() 
{
	delete []mRecvBuffer;
}

void JZNetDataManager::newSession(int sessionId, QTcpSocket *socket)
{	
	m_sessionInfo[sessionId].socket = socket;	
}

void JZNetDataManager::endSession(int sessionId)
{	
	m_sessionInfo.remove(sessionId);	
}

QTcpSocket *JZNetDataManager::socket(int sessionId)
{
    return m_sessionInfo[sessionId].socket;
}

void JZNetDataManager::recvPack(int sessionId)
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

void JZNetDataManager::recvData(int sessionId, QByteArray data)
{
	m_sessionInfo[sessionId].buffer.append(data);
	parsePack(sessionId);
}

QByteArray JZNetDataManager::packData(const JZNetPack *pack)
{	
	QByteArray send_buffer;	
	QDataStream s(&send_buffer,QIODevice::WriteOnly);		

	int tmp = 0;
    int seq = pack->seq();
    int type = pack->type();
	s.writeRawData((char*)&packHead,sizeof(packHead));
    s.writeRawData((char*)&seq,sizeof(int));
    s.writeRawData((char*)&type,sizeof(int));
    s.writeRawData((char*)&tmp,sizeof(int));
	pack->saveToStream(s);
	s.writeRawData((char*)&packTail,sizeof(packTail));

    int *ptr = (int*)(send_buffer.data() + 1 + 8);
    *ptr = send_buffer.size() - 12 - 2;

	return send_buffer;
}

bool JZNetDataManager::sendPack(int sessionId, JZNetPack *pack)
{		
	if(pack->seq() == -1)
		pack->setSeq(m_packSeq++);

	QByteArray send_buffer = packData(pack);
	QTcpSocket *socket = m_sessionInfo[sessionId].socket;
	Q_ASSERT(socket);    
        
    const char *ptr = send_buffer.data();
    int size = send_buffer.size();    
    int write_size = socket->write(ptr,size);    
    if(write_size == size)
        return socket->waitForBytesWritten();
    else
        return false;
}

JZNetPackPtr JZNetDataManager::takePack(int sessionId,int type,int param)
{	
	JZNetPackPtr pack;
	QVector<JZNetPackPtr> &pack_list = m_sessionInfo[sessionId].packList;
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
                if((type == 1 && pack_list[i]->type() == param)
                   || (type == 2 && pack_list[i]->seq() == param))
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

JZNetPackPtr JZNetDataManager::takePack(int sessionId)
{
	return takePack(sessionId, 0, 0);
}

JZNetPackPtr JZNetDataManager::takePackByType(int sessionId, int type)
{
	return takePack(sessionId, 1, type);
}

JZNetPackPtr JZNetDataManager::takePackBySeq(int sessionId, int seq)
{
	return takePack(sessionId, 2, seq);
}

void JZNetDataManager::parsePack(int sessionId)
{
	QByteArray &buffer = m_sessionInfo[sessionId].buffer;	
	
    int min_pack_head = 12; //seq type len
	int min_pack_len = 1 + min_pack_head + 1;		
	int buffer_start = 0;
	while (buffer.length() - buffer_start >= min_pack_len)
	{
		int start = buffer.indexOf(packHead, buffer_start);		
		if (start < 0)
		{			
			buffer_start = buffer.length(); //全是无用数据，清除
			break;
		}
        start += 1;
        if (buffer.length() < start + min_pack_head + 1)
			break;
		
        int pack_seq = *((int*)(buffer.data() + start));
        int pack_type = *((int*)(buffer.data() + start + 4));
        int body_size = *((int*)(buffer.data() + start + 8));
        //放外网被攻击
        if (body_size < 0 || pack_type < 0 || pack_seq < 0)
        {
            buffer_start = buffer.length(); //全是无用数据，清除
            break;
        }

        start += 12;
        if (buffer.length() < start + body_size + 1)
			break;
		
		char tail = *((int*)(buffer.data() + start + body_size));
		if(tail != packTail)		
		{			
            buffer_start = start + body_size + 1;  //结尾不对,跳过这个数据包
			break;
		}
		
		QByteArray pack_buffer = buffer.mid(start, body_size);
		QDataStream s(&pack_buffer,QIODevice::ReadOnly);
		
		//放外网被攻击
		JZNetPack *pack = JZNetPackManager::instance()->createPack(pack_type);
		if(!pack)
        {
            buffer_start = start + body_size + 1;
			break;
        }
		
        pack->setSeq(pack_seq);
		pack->loadFromStream(s);
		m_sessionInfo[sessionId].packList.push_back(JZNetPackPtr(pack));
		start += body_size + 1;

		//下一次读起始
		buffer_start = start;
	}
	if(buffer_start != 0)
		buffer = buffer.mid(buffer_start);
}
