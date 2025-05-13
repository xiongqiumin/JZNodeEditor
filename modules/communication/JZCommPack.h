#ifndef JZ_COMM_PACK_H_
#define JZ_COMM_PACK_H_

#include <QByteArray>

class JZCommPackFormat
{
public:
	JZCommPackFormat();

	void setHeader(QByteArray head);
	void addPackType(int size);
	void addPackSize(int size);
	void addCheckSum(int type, int size);
	void setTail(QByteArray head);

	QByteArray head;
	QByteArray tail;
};

class JZCommPack
{
public:
	JZCommPack();

	void setFormat(JZCommPackFormat format);

	void appendBuffer(QByteArray buffer);
	bool takePack(QByteArray& pack);
	QByteArray makePack(QByteArray body);

protected:
	JZCommPackFormat m_format;

	QByteArray m_buffer;
};

#endif // !JZ_COMM_PACK_H_

