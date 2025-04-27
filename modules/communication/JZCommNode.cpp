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
    setPinTypeInt(in1);

    int in2 = addParamIn("value");
    setPinType(in2, { "uint16"  } );
    
    setParamInValue(0, "40000");
    setParamInValue(1, "1");
    m_readType = "uint16";

	addParamOut("result");

	m_commIndex = 0;
	m_function = 0;	
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

void JZNodeModbusRead::setReadType(QString type)
{
	m_readType = type;
}

QString JZNodeModbusRead::readType()
{
	return m_readType;
}

bool JZNodeModbusRead::updateNode(QString& error)
{
	int func_type = m_function;
	setPinType(paramOut(0), { m_readType });
	return true;
}

bool JZNodeModbusRead::compiler(JZNodeCompiler* c, QString& error)
{
	auto env = c->env();
    if (!c->addFlowInput(m_id, error))
        return false;

	if (!c->checkVariableType("this.commManager", env->nameToType("JZCommManager"), error))
		return false;

	int client_id = c->allocStack("JZModbusClient*");
	JZNodeIRParam client = irId(client_id);

	QList<JZNodeIRParam> get_in, get_out;
	get_in << irRef("this.commManager") << irLiteral(m_commIndex);
	get_out << client;
	c->addCallConvert("JZCommManager::modbusClient", get_in, get_out);

	QJsonObject obj;
	
	JZNodeIRParam ir_addr = irId(c->paramId(m_id, paramIn(0)));	
	JZNodeIRParam ir_out = irId(c->paramId(m_id, paramOut(0)));

    int json = c->allocStack(Type_jsonObject);
    c->addSetJson(irId(json), "function", irLiteral(m_function));
    c->addSetJson(irId(json), "addr", ir_addr);

    int any_id = c->allocStack(Type_any);

    QList<JZNodeIRParam> read_in, read_out;
    read_in << client << irId(json);
    read_out << irId(any_id);
    c->addCallConvert("JZCommModbusRead", read_in, read_out);
    c->addConvert(ir_out, env->nameToType(m_readType), irId(any_id));

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
    m_type = Node_ModbusWrite;
    m_name = "ModbusWrite";

    addFlowIn();
    addFlowOut();
}
JZNodeModbusWrite::~JZNodeModbusWrite()
{
}

bool JZNodeModbusWrite::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

	return true;
}

void JZNodeModbusWrite::saveToStream(QDataStream& s) const
{
	JZNode::saveToStream(s);
    s << m_commIndex;
    s << m_function;
}

void JZNodeModbusWrite::loadFromStream(QDataStream& s)
{
	JZNode::loadFromStream(s);        
    s >> m_commIndex;
    s >> m_function;
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

