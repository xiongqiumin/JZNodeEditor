#include <QDebug>
#include "JZCommPack.h"

JZCommPack::JZCommPack()
{
}

void JZCommPack::addHeader(QByteArray head) 
{
	m_head = head;
}

void JZCommPack::addPackType(int size) 
{
}

void JZCommPack::addPackSize(int size) 
{
}

void JZCommPack::addCheckSum(int type, int size) 
{
}

void JZCommPack::addTail(QByteArray tail)
{
	m_tail = tail;
}

void JZCommPack::appendBuffer(QByteArray buffer)
{
	m_buffer.append(buffer);
}

bool JZCommPack::takePack(QByteArray& pack)
{
	int min_pack_len = m_head.size() + m_tail.size();
	
	bool get_pack = false;
	int buffer_start = 0;
	while (m_buffer.length() - buffer_start >= min_pack_len)
	{
		int start = 0;
		if (!m_head.isEmpty())
		{
			m_buffer.indexOf(m_head, buffer_start);
			if (start < 0)
			{
				qDebug() << "invaild packet data.";
				buffer_start = m_buffer.length();  //全是无用数据，清除
				break;
			}

			start += m_head.size();
		}
		
		if (!m_tail.isEmpty())
		{
			int tail_end = m_buffer.indexOf(m_tail, start);
			if (tail_end >= 0)
			{
				get_pack = true;
				pack = m_buffer.mid(start, tail_end - start);
				buffer_start = tail_end + m_tail.size();
			}
			break;
		}
	}
	if (buffer_start != 0)
		m_buffer = m_buffer.mid(buffer_start);

	return get_pack;
}

QByteArray JZCommPack::makePack(QByteArray body)
{
	QByteArray buffer;
	if (!m_head.isEmpty())
		buffer.append(m_head);

	buffer.append(body);

	if (!m_tail.isEmpty())
		buffer.append(m_tail);

	return buffer;
}