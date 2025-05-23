#include <QDebug>
#include "JZCommPack.h"

//JZCommPackFormat
JZCommPackFormat::JZCommPackFormat()
{
	tail.push_back('\n');
}

void JZCommPackFormat::setHeader(QByteArray head)
{
	head = head;
}

void JZCommPackFormat::addPackType(int size)
{
}

void JZCommPackFormat::addPackSize(int size)
{
}

void JZCommPackFormat::addCheckSum(int type, int size)
{
}

void JZCommPackFormat::setTail(QByteArray tail)
{
	tail = tail;
}

//JZCommConfigPtr
QDataStream& operator<<(QDataStream& s, const JZCommPackFormat& param)
{
	return s;
}

QDataStream& operator>>(QDataStream& s, JZCommPackFormat& param)
{
	return s;
}

//JZCommPack
JZCommPack::JZCommPack()
{
}

void JZCommPack::setFormat(JZCommPackFormat format)
{
	m_format = format;
}

void JZCommPack::appendBuffer(QByteArray buffer)
{
	m_buffer.append(buffer);
}

bool JZCommPack::takePack(QByteArray& pack)
{
	auto& head = m_format.head;
	auto& tail = m_format.tail;

	int min_pack_len = head.size() + tail.size();
	bool get_pack = false;
	int buffer_start = 0;
	while (m_buffer.length() - buffer_start >= min_pack_len)
	{
		int start = 0;
		if (!head.isEmpty())
		{
			m_buffer.indexOf(head, buffer_start);
			if (start < 0)
			{
				qDebug() << "invaild packet data.";
				buffer_start = m_buffer.length();  //全是无用数据，清除
				break;
			}

			start += head.size();
		}
		
		if (!tail.isEmpty())
		{
			int tail_end = m_buffer.indexOf(tail, start);
			if (tail_end >= 0)
			{
				get_pack = true;
				pack = m_buffer.mid(start, tail_end - start);
				buffer_start = tail_end + tail.size();
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
	if (!m_format.head.isEmpty())
		buffer.append(m_format.head);

	buffer.append(body);

	if (!m_format.tail.isEmpty())
		buffer.append(m_format.tail);

	return buffer;
}