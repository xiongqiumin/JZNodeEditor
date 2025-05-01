#include "JZCommNode.h"
#include "JZNodeCompiler.h"
#include "JZCommManager.h"
#include "JZNodeUtils.h"

bool checkCommManager(int id, JZNodeCompiler* c, QString& error)
{
	if (!c->addFlowInput(id, error))
		return false;

	auto env = c->env();
	if(!c->checkVariableType("this.commManager",env->nameToType("JZCommManager"), error))
		return false;

	return true;
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

void JZNodeCommInit::setConfig(JZCommManagerConfig config)
{
	m_config = config;
}

JZCommManagerConfig JZNodeCommInit::config()
{
	return m_config;
}

bool JZNodeCommInit::compiler(JZNodeCompiler* c, QString& error)
{
	if (!checkCommManager(m_id, c, error))
		return false;

	int id = c->allocStack(Type_byteArray);
	c->addSetBuffer(irId(id), JZNodeUtils::toBuffer(m_config));

	QList<JZNodeIRParam> in, out;
	in << irRef("this.commManager") << irId(id);
	c->addCallConvert("JZCommInit",in,out);
	return true;
}

void JZNodeCommInit::saveToStream(QDataStream& s) const
{
    JZNode::saveToStream(s);
    s << m_config;
}

void JZNodeCommInit::loadFromStream(QDataStream& s)
{
    JZNode::loadFromStream(s);
    s >> m_config;
}

//JZNodeModbusRead
JZNodeModbusRead::JZNodeModbusRead()
{
    m_type = Node_ModbusRead;
    m_name = "ModbusRead";    

	addFlowIn();
	addFlowOut();

	m_modbus = "modbus";
	m_dataType = "uint16";
	m_function = Function_Register;

	int in1 = addParamIn("addr");
    setPinTypeInt(in1);

    setParamInValue(0, "40000");
	addParamOut("result");
}

JZNodeModbusRead::~JZNodeModbusRead()
{
}

void JZNodeModbusRead::setClient(QString comm)
{
	m_modbus = comm;
}

QString JZNodeModbusRead::client()
{
	return m_modbus;
}

void JZNodeModbusRead::setFunction(int function)
{
	m_function = function;
	if (m_function == Function_Bit || m_function == Function_InputBit)
		m_dataType = "uint8";

	update();
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

void JZNodeModbusRead::setDataType(QString type)
{
	m_dataType = type;
	update();
}

QString JZNodeModbusRead::dataType()
{
	return m_dataType;
}

bool JZNodeModbusRead::updateNode(QString& error)
{
	int func_type = m_function;
	setPinType(paramOut(0), { m_dataType });
	return true;
}

bool JZNodeModbusRead::compiler(JZNodeCompiler* c, QString& error)
{
	if (!checkCommManager(m_id,c,error))
		return false;

	auto env = c->env();
	JZNodeIRParam ir_addr = irId(c->paramId(m_id, paramIn(0)));	
	JZNodeIRParam ir_out = irId(c->paramId(m_id, paramOut(0)));

    int json = c->allocStack(Type_jsonObject);
	c->addInitVariable(irId(json),Type_jsonObject);
    c->addSetJson(irId(json), "function", irLiteral(m_function));
    c->addSetJson(irId(json), "addr", ir_addr);
	c->addSetJson(irId(json), "dataType", irLiteral(m_dataType));

    int any_id = c->allocStack(Type_any);

    QList<JZNodeIRParam> read_in, read_out;
    read_in << irRef("this.commManager")  << irLiteral(m_modbus) << irId(json);
    read_out << irId(any_id);
    c->addCallConvert("JZCommModbusRead", read_in, read_out);
    c->addConvert(ir_out, env->nameToType(m_dataType), irId(any_id));

	return true;
}

void JZNodeModbusRead::saveToStream(QDataStream& s) const
{
	JZNode::saveToStream(s);
	s << m_modbus;
	s << m_function;
	s << m_dataType;
}

void JZNodeModbusRead::loadFromStream(QDataStream& s)
{
	JZNode::loadFromStream(s);
	s >> m_modbus;
	s >> m_function;
	s >> m_dataType;
}


//JZNodeModbusWrite
JZNodeModbusWrite::JZNodeModbusWrite()
{
    m_type = Node_ModbusWrite;
    m_name = "ModbusWrite";

    addFlowIn();
    addFlowOut();

	int in1 = addParamIn("addr");
	setPinTypeInt(in1);

	int in2 = addParamIn("value");
	setPinType(in2, { m_dataType });

	setParamInValue(0, "40000");
	
	m_dataType = "uint16";
	m_function = Function_Register;
}
JZNodeModbusWrite::~JZNodeModbusWrite()
{
}

bool JZNodeModbusWrite::compiler(JZNodeCompiler* c, QString& error)
{
	if (!checkCommManager(m_id,c,error))
		return false;

	JZNodeIRParam ir_addr = irId(c->paramId(m_id, paramIn(0)));
	JZNodeIRParam ir_value = irId(c->paramId(m_id, paramIn(1)));

	int json = c->allocStack(Type_jsonObject);
	c->addInitVariable(irId(json), Type_jsonObject);
	c->addSetJson(irId(json), "function", irLiteral(m_function));
	c->addSetJson(irId(json), "addr", ir_addr);
	c->addSetJson(irId(json), "dataType", irLiteral(m_dataType));

	QList<JZNodeIRParam> write_in, write_out;
	write_in << irRef("this.commManager") << irLiteral(m_modbus) << irId(json) << ir_value;
	c->addCallConvert("JZCommModbusWrite", write_in, write_out);

	return true;
}

void JZNodeModbusWrite::setClient(QString comm)
{
	m_modbus = comm;
}

QString JZNodeModbusWrite::client()
{
	return m_modbus;
}

void JZNodeModbusWrite::setFunction(int function)
{
	m_function = function;
	if (m_function == Function_Bit || m_function == Function_InputBit)
		m_dataType = "uint8";

	update();
}

int JZNodeModbusWrite::function()
{
	return m_function;
}

void JZNodeModbusWrite::setAddr(int addr)
{
	setParamInValue(0, QString::number(addr));
}

int JZNodeModbusWrite::addr()
{
	return paramInValue(0).toInt();
}

void JZNodeModbusWrite::setValue(QString value)
{
    setParamInValue(1, value);
}

QString JZNodeModbusWrite::value()
{
    return paramInValue(1);
}

void JZNodeModbusWrite::setDataType(QString type)
{
	m_dataType = type;
	update();
}

QString JZNodeModbusWrite::dataType()
{
	return m_dataType;
}

bool JZNodeModbusWrite::updateNode(QString& error)
{
	int func_type = m_function;
	setPinType(paramIn(1), { m_dataType });
	return true;
}

void JZNodeModbusWrite::saveToStream(QDataStream& s) const
{
	JZNode::saveToStream(s);
    s << m_modbus;
    s << m_function;
	s << m_dataType;
}

void JZNodeModbusWrite::loadFromStream(QDataStream& s)
{
	JZNode::loadFromStream(s);        
    s >> m_modbus;
    s >> m_function;
	s >> m_dataType;
}

//JZNodeTcpClientRead
JZNodeTcpClientRead::JZNodeTcpClientRead()
{
	m_type = Node_TcpClientRead;
	m_name = "TcpClientRead";
}
JZNodeTcpClientRead::~JZNodeTcpClientRead()
{
}

bool JZNodeTcpClientRead::compiler(JZNodeCompiler* c, QString& error) 
{
	if (!checkCommManager(m_id,c,error))
		return false;

	return true;
}

void JZNodeTcpClientRead::saveToStream(QDataStream& s) const 
{
	JZNode::saveToStream(s);
}

void JZNodeTcpClientRead::loadFromStream(QDataStream& s) 
{    
	JZNode::loadFromStream(s);
}

//JZNodeTcpClientWrite
JZNodeTcpClientWrite::JZNodeTcpClientWrite() 
{
	m_type = Node_TcpClientWrite;
	m_name = "TcpClientWrite";
}

JZNodeTcpClientWrite::~JZNodeTcpClientWrite() 
{
}

bool JZNodeTcpClientWrite::compiler(JZNodeCompiler* c, QString& error) 
{
	if (!checkCommManager(m_id,c,error))
		return false;

	return true;
}

void JZNodeTcpClientWrite::saveToStream(QDataStream& s) const 
{
	JZNode::saveToStream(s);
}

void JZNodeTcpClientWrite::loadFromStream(QDataStream& s) 
{
	JZNode::loadFromStream(s);
}


//JZNodeUdpRead
JZNodeUdpRead::JZNodeUdpRead() 
{
	m_type = Node_UdpRead;
	m_name = "UdpRead";
}
JZNodeUdpRead::~JZNodeUdpRead() 
{
}

bool JZNodeUdpRead::compiler(JZNodeCompiler* c, QString& error)
{
	if (!checkCommManager(m_id,c,error))
		return false;

	return true;
}

void JZNodeUdpRead::saveToStream(QDataStream& s) const 
{
	JZNode::saveToStream(s);
}

void JZNodeUdpRead::loadFromStream(QDataStream& s)
{
	JZNode::loadFromStream(s);
}


//JZNodeUdpWrite
JZNodeUdpWrite::JZNodeUdpWrite()
{
	m_type = Node_UdpWrite;
	m_name = "UdpWrite";
}
JZNodeUdpWrite::~JZNodeUdpWrite()
{
}

bool JZNodeUdpWrite::compiler(JZNodeCompiler* c, QString& error)
{
	if (!checkCommManager(m_id,c,error))
		return false;

	return true;
}

void JZNodeUdpWrite::saveToStream(QDataStream& s) const 
{
	JZNode::saveToStream(s);
}

void JZNodeUdpWrite::loadFromStream(QDataStream& s) 
{
	JZNode::loadFromStream(s);
}


//JZNodeSerialRead
JZNodeSerialRead::JZNodeSerialRead() 
{
	m_type = Node_SerialRead;
	m_name = "SerialRead";
}
JZNodeSerialRead::~JZNodeSerialRead()
{
}

bool JZNodeSerialRead::compiler(JZNodeCompiler* c, QString& error) 
{
	if (!checkCommManager(m_id,c,error))
		return false;

	return true;
}

void JZNodeSerialRead::saveToStream(QDataStream& s) const 
{
	JZNode::saveToStream(s);
}

void JZNodeSerialRead::loadFromStream(QDataStream& s) 
{
	JZNode::loadFromStream(s);
}

//JZNodeSerialWrite
JZNodeSerialWrite::JZNodeSerialWrite() 
{
	m_type = Node_SerialWrite;
	m_name = "SerialWrite";
}
JZNodeSerialWrite::~JZNodeSerialWrite() 
{
}

bool JZNodeSerialWrite::compiler(JZNodeCompiler* c, QString& error) 
{
	if (!checkCommManager(m_id,c,error))
		return false;
		
	return true;
}

void JZNodeSerialWrite::saveToStream(QDataStream& s) const
{
	JZNode::saveToStream(s);
}

void JZNodeSerialWrite::loadFromStream(QDataStream& s)
{
	JZNode::loadFromStream(s);
}

