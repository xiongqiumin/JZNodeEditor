#ifndef JZ_COMM_PACK_H_
#define JZ_COMM_PACK_H_

#include <QByteArray>

class JZCommPack
{
public:
	JZCommPack();

	void addHeader(QByteArray head);
	void addPackType(int size);
	void addPackSize(int size);
	void addCheckSum(int type, int size);
	void addTail(QByteArray head);

	void appendBuffer(QByteArray buffer);
	bool takePack(QByteArray& pack);

	QByteArray makePack(QByteArray body);

protected:
	QByteArray m_head;
	QByteArray m_tail;

	QByteArray m_buffer;
};

#endif // !JZ_COMM_PACK_H_

