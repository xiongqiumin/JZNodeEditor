#include "JZCommNode.h"
#include "JZNodeCompiler.h"
#include "JZCommManager.h"
#include "JZNodeJson.h"


JZModbusClient* JZModbusGet(JZCommManager* manager, int index)
{
	JZModbusClient* client = manager->modbusClient(index);
	if (!client)
		throw std::runtime_error("no client");

	if (!client->isOpen())
		throw std::runtime_error("write failed");
	
	return client;
}

//JZNodeCommInit
JZNodeCommInit::JZNodeCommInit()
{
	m_type = Node_CommInit;
	m_name = "commInit";

	addFlowIn();
	addFlowOut();
}

JZNodeCommInit::~JZNodeCommInit()
{
}

bool JZNodeCommInit::compiler(JZNodeCompiler* c, QString& error)
{
	c->addNodeEnter(m_id);

	auto env = c->env();
	if(!c->checkVariableType("this.commManager",env->nameToType("JZCommManager"), error))
		return false;

	int id = c->allocStack(Type_byteArray);
	c->addSetBuffer(irId(id), QByteArray());

	QList<JZNodeIRParam> in, out;
	in << irRef("this.commManager") << irId(id);
	c->addCallConvert("JZCommInit",in,out);
	return true;
}

//JZNodeModbusRead
JZNodeModbusRead::JZNodeModbusRead()
{
	addFlowIn();
	addFlowOut();

	int in1 = addParamIn("addr");
	int in2 = addParamIn("count");
	setPinTypeInt(in1);
	setPinTypeInt(in2);

	addParamOut("result");

	m_commIndex = 0;
	m_function = 0;
	m_bitOrder = 0;
}

JZNodeModbusRead::~JZNodeModbusRead()
{
}

int JZNodeModbusRead::readTypeSize()
{
	return JZNodeType::byteSize(m_readType);
}

void JZNodeModbusRead::setCommIndex(int comm)
{
	m_commIndex = comm;
}

int JZNodeModbusRead::readComm()
{
	return m_commIndex;
}

void JZNodeModbusRead::setFunction(int function)
{
	m_function = function;
}

int JZNodeModbusRead::function()
{
	return m_function;
}

void JZNodeModbusRead::setAddr(int addr)
{
	setParamInValue(0, QString::number(addr));
}

int JZNodeModbusRead::addr()
{
	return paramInValue(0).toInt();
}

void JZNodeModbusRead::setCount(int count)
{
	setParamInValue(1, QString::number(count));
}

int JZNodeModbusRead::count()
{
	return paramInValue(1).toInt();
}

void JZNodeModbusRead::setReadType(QString type)
{
	m_readType = type;
}

QString JZNodeModbusRead::readType()
{
	return m_readType;
}

void JZNodeModbusRead::setBitOrder(int order)
{
	m_bitOrder = order;
}

int JZNodeModbusRead::bitOrder()
{
	return m_bitOrder;
}

bool JZNodeModbusRead::updateNode(QString& error)
{
	int func_type = m_function;
	if (func_type == readBit || func_type == readInputBit || func_type == readBits || func_type == readInputBits)
	{
		setPinType(paramOut(0), { "QList<" + m_readType  + ">" });
	}
	else if (func_type == readInputRegister || func_type == readRegister)
	{
		setPinType(paramOut(0), { m_readType });
	}

	return true;
}

bool JZNodeModbusRead::compiler(JZNodeCompiler* c, QString& error)
{
	auto env = c->env();
	c->addNodeEnter(m_id);

	if (c->checkVariableType("commManager", env->nameToType("JZCommManager"), error))
		return false;

	int client_id = c->allocStack("JZModbusClient*");
	JZNodeIRParam client = irId(client_id);

	QList<JZNodeIRParam> get_in, get_out;
	get_in << irRef("this.commManager") << irLiteral(m_commIndex);
	get_out << client;
	c->addCall("JZNodeCommInit", get_in, get_out);

	QJsonObject obj;
	
	JZNodeIRParam ir_addr = irId(c->paramId(m_id, paramIn(0)));
	JZNodeIRParam ir_count = irId(c->paramId(m_id, paramIn(1)));
	JZNodeIRParam ir_out = irId(c->paramId(m_id, paramOut(0)));

	bool need_trans = false;
	bool need_get = false;

	int func_type = m_function;
	QString from_type = readTypeSize() == 1? "uint8":"uint16";
	if (func_type == readBit || func_type == readInputBit 
		|| func_type == readInputRegister || func_type == readRegister)
	{
		ir_count = irLiteral(readTypeSize());
		need_get = true;
	}
	else
	{
		c->addExpr(ir_count, ir_count, irLiteral(readTypeSize()), OP_mul);
	}
	
	QList<JZNodeIRParam> in, out;
	in << client << ir_addr << ir_count;

	JZNodeIRParam read_out;

	QString read_list_type = "QList<" + from_type + ">";
	QString list_type = "QList<" + m_readType + ">";
	QString trasFunc = "JZTransList" + from_type + "to" + m_readType;

	int read_out_id = c->allocStack(read_list_type);
	read_out = irId(read_out_id);
	out << read_out;

	QList<JZNodeIRParam> list_get_in, list_get_out;
	list_get_in << read_out;
	list_get_out << ir_out;

	if (func_type == readBit || func_type == readBits)
		c->addCall("JZModbusClient::readBits", in, out);
	else if (func_type == readInputBit || func_type == readInputBits)
		c->addCall("JZModbusClient::readInputBits", in, out);
	else if (func_type == readInputRegister || func_type == readInputRegisters)
		c->addCall("JZModbusClient::readInputRegisters", in, out);
	else if (func_type == readRegister || func_type == readRegisters)
		c->addCall("JZModbusClient::readRegisters", in, out);

	if (from_type != m_readType
		|| (from_type == "uint16" && m_bitOrder == QDataStream::BigEndian))
	{
		QList<JZNodeIRParam> trans_in, trans_out;
		trans_in << read_out << irLiteral(m_bitOrder);

		if (need_get)
		{
			int read_out_id = c->allocStack(read_list_type);
			trans_out << irId(read_out_id);
			list_get_in[0] = irId(read_out_id);
		}
		else
		{
			trans_out << ir_out;
		}

		c->addCall(trasFunc, trans_in, trans_out);
	}
	if (need_get)
		c->addCall(list_type + "::get", list_get_in, list_get_out);

	return true;
}

void JZNodeModbusRead::saveToStream(QDataStream& s) const
{
	JZNode::saveToStream(s);
	s << m_commIndex;
	s << m_function;
}

void JZNodeModbusRead::loadFromStream(QDataStream& s)
{
	JZNode::loadFromStream(s);
	s >> m_commIndex;
	s >> m_function;
}


//JZNodeModbusWrite
JZNodeModbusWrite::JZNodeModbusWrite()
{
}
JZNodeModbusWrite::~JZNodeModbusWrite()
{
}

bool JZNodeModbusWrite::compiler(JZNodeCompiler* compiler, QString& error)
{
	return true;
}

void JZNodeModbusWrite::saveToStream(QDataStream& s) const
{
	JZNode::saveToStream(s);
}

void JZNodeModbusWrite::loadFromStream(QDataStream& s)
{
	JZNode::loadFromStream(s);
}

void JZModbusWrite(JZCommManager* manager, const QByteArray& buffer)
{
	auto obj = JZNodeJson::formBuffer(buffer);

	JZModbusConnetInfo conn;
	int idx = manager->indexOfModbusClient(conn);

	JZModbusClient* client = nullptr;
	if (idx == -1)
	{
		client = manager->newModbusClient(conn);
		client->open();
	}
	else
		client = manager->modbusClient(idx);

	if (!client->isOpen())
	{
		throw std::runtime_error("write failed");
		return;
	}

	int type = obj["type"].toInt();
	int addr = obj["addr"].toInt();

	bool ret = false;
	if (type == 0)
	{
		int value = JZNodeJson::getValue<int>(obj, "value");
		ret = client->writeBit(addr, value);
	}
	else if (type == 1)
	{
		auto value = JZNodeJson::getValue<QVector<uint8_t>>(obj, "value");
		ret = client->writeBits(addr, value);
	}
	else if (type == 2)
	{
		auto value = JZNodeJson::getValue<int>(obj, "value");
		ret = client->writeRegister(addr, value);
	}
	else if (type == 3)
	{
		auto value = JZNodeJson::getValue<QVector<uint16_t>>(obj, "value");
		ret = client->writeRegisters(addr, value);
	}
	if (!ret)
		throw std::runtime_error("write failed");
}

//JZNodeTcpClientRead
JZNodeTcpClientRead::JZNodeTcpClientRead()
{
}
JZNodeTcpClientRead::~JZNodeTcpClientRead()
{
}

bool JZNodeTcpClientRead::compiler(JZNodeCompiler* compiler, QString& error) 
{
	return true;
}

void JZNodeTcpClientRead::saveToStream(QDataStream& s) const 
{
}

void JZNodeTcpClientRead::loadFromStream(QDataStream& s) 
{
}

//JZNodeTcpClientWrite
JZNodeTcpClientWrite::JZNodeTcpClientWrite() 
{
}
JZNodeTcpClientWrite::~JZNodeTcpClientWrite() 
{
}

bool JZNodeTcpClientWrite::compiler(JZNodeCompiler* compiler, QString& error) 
{
	return true;
}

void JZNodeTcpClientWrite::saveToStream(QDataStream& s) const 
{
}
void JZNodeTcpClientWrite::loadFromStream(QDataStream& s) 
{
}


//JZNodeUdpRead
JZNodeUdpRead::JZNodeUdpRead() 
{
}
JZNodeUdpRead::~JZNodeUdpRead() 
{
}

bool JZNodeUdpRead::compiler(JZNodeCompiler* compiler, QString& error)
{
	return true;
}

void JZNodeUdpRead::saveToStream(QDataStream& s) const 
{
}
void JZNodeUdpRead::loadFromStream(QDataStream& s)
{
}


//JZNodeUdpWrite
JZNodeUdpWrite::JZNodeUdpWrite()
{
}
JZNodeUdpWrite::~JZNodeUdpWrite()
{
}

bool JZNodeUdpWrite::compiler(JZNodeCompiler* compiler, QString& error)
{
	return true;
}

void JZNodeUdpWrite::saveToStream(QDataStream& s) const 
{
}
void JZNodeUdpWrite::loadFromStream(QDataStream& s) 
{
}


//JZNodeSerialRead
JZNodeSerialRead::JZNodeSerialRead() 
{
}
JZNodeSerialRead::~JZNodeSerialRead()
{
}

bool JZNodeSerialRead::compiler(JZNodeCompiler* compiler, QString& error) 
{
	return true;
}

void JZNodeSerialRead::saveToStream(QDataStream& s) const 
{
}
void JZNodeSerialRead::loadFromStream(QDataStream& s) 
{
}

//JZNodeSerialWrite
JZNodeSerialWrite::JZNodeSerialWrite() 
{
}
JZNodeSerialWrite::~JZNodeSerialWrite() 
{
}

bool JZNodeSerialWrite::compiler(JZNodeCompiler* compiler, QString& error) 
{
	return true;
}

void JZNodeSerialWrite::saveToStream(QDataStream& s) const
{
}
void JZNodeSerialWrite::loadFromStream(QDataStream& s)
{
}

