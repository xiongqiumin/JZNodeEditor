#include "JZCommNode.h"
#include "JZNodeCompiler.h"
#include "JZCommManager.h"
#include "JZNodeUtils.h"

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
    if (!c->addFlowInput(m_id, error))
        return false;

    auto env = c->env();
    if (!c->checkVariableType("this.commManager", env->nameToType("JZCommManager"), error))
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

//JZCommNode
JZCommNode::JZCommNode()
{
    addFlowIn();
    addFlowOut();

    int in = addParamIn("name", Pin_constValue | Pin_noCompiler);
    setPinTypeString(in);
}

JZCommNode::~JZCommNode()
{
}

void JZCommNode::setName(QString name)
{
    setParamInValue(0, name);
}

QString JZCommNode::name()
{
    return paramInValue(0);
}

QList<JZNodeIRParam> JZCommNode::toParamId(QList<JZNodeIRParam> ir_list)
{
	QList<JZNodeIRParam> out = ir_list;
	for (int i = 0; i < out.size(); i++)
	{
		if (out[i].isStack() && out[i].id() < MAX_PIN_ID)
			out[i].m_id = JZNodeGemo::paramId(m_id, out[i].m_id);
	}
	return out;
}

bool JZCommNode::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    auto env = c->env();
    if (!c->checkVariableType("this.commManager", env->nameToType("JZCommManager"), error))
        return false;
    
	QList<JZNodeIRParam> ir_in = toParamId(m_input);
	QList<JZNodeIRParam> ir_out = toParamId(m_output);

	QList<JZNodeIRParam> in;
	in << irRef("this.commManager");
	in << irLiteral(name());
	in << ir_in;
	c->addCallConvert(m_function, m_input, ir_out);
    return true;
}


//JZNodeModbusRead
JZNodeModbusRead::JZNodeModbusRead()
{
    m_type = Node_ModbusRead;
    m_name = "ModbusRead";    	

	m_modbus = "modbus";
	m_dataType = "uint16";
	m_modbusFunc = Function_Register;

	int in1 = addParamIn("addr");
    setPinTypeInt(in1);

    setPinValue(in1, "40000");
	int out = addParamOut("result");

	m_function = "JZCommModbusRead";
	m_input << irLiteral(0);
	m_input << irLiteral(0);
	m_input << irId(in1);
	m_output << irId(out);
}

JZNodeModbusRead::~JZNodeModbusRead()
{
}

bool JZNodeModbusRead::compiler(JZNodeCompiler* compiler, QString& error)
{
	m_input[0] = irLiteral(m_modbusFunc);
	m_input[1] = irLiteral(m_dataType);
	return JZCommNode::compiler(compiler, error);
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
	m_modbusFunc = function;
	if (m_modbusFunc == Function_Bit || m_modbusFunc == Function_InputBit)
		m_dataType = "uint8";
	else
		m_dataType = "uint16";

	update();
}

int JZNodeModbusRead::function()
{
	return m_modbusFunc;
}

void JZNodeModbusRead::setAddr(int addr)
{
	setParamInValue(1, QString::number(addr));
}

int JZNodeModbusRead::addr()
{
	return paramInValue(1).toInt();
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
	int func_type = m_modbusFunc;
	setPinType(paramOut(0), { m_dataType });
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
    m_dataType = "uint16";
	m_modbusFunc = Function_Register;

	int in1 = addParamIn("addr");
	setPinTypeInt(in1);    

	int in2 = addParamIn("value");
	setPinType(in2, { m_dataType });
    setPinValue(in1, "40000");		

	m_function = "JZCommModbusWrite";
	m_input << irLiteral(0);
	m_input << irLiteral(0);
	m_input << irId(in1);
	m_input << irId(in2);
}

JZNodeModbusWrite::~JZNodeModbusWrite()
{
}

bool JZNodeModbusWrite::compiler(JZNodeCompiler* compiler, QString& error)
{
	m_input[0] = irLiteral(m_modbusFunc);
	m_input[1] = irLiteral(m_dataType);
	return JZCommNode::compiler(compiler, error);
}


void JZNodeModbusWrite::setFunction(int function)
{
	m_modbusFunc = function;
	if (m_modbusFunc == Function_Bit || m_modbusFunc == Function_InputBit)
		m_dataType = "uint8";
	else
		m_dataType = "uint16";

	update();
}

int JZNodeModbusWrite::function()
{
	return m_modbusFunc;
}

void JZNodeModbusWrite::setAddr(int addr)
{
	setParamInValue(1, QString::number(addr));
}

int JZNodeModbusWrite::addr()
{
	return paramInValue(1).toInt();
}

void JZNodeModbusWrite::setValue(QString value)
{
    setParamInValue(2, value);
}

QString JZNodeModbusWrite::value()
{
    return paramInValue(2);
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
	setPinType(paramIn(2), { m_dataType });
	return true;
}

void JZNodeModbusWrite::saveToStream(QDataStream& s) const
{
	JZNode::saveToStream(s);
    s << m_function;
	s << m_dataType;
}

void JZNodeModbusWrite::loadFromStream(QDataStream& s)
{
	JZNode::loadFromStream(s);        
    s >> m_function;
	s >> m_dataType;
}

//JZNodeTcpClientRead
JZNodeTcpClientRead::JZNodeTcpClientRead()
{
	m_type = Node_TcpClientRead;
	m_name = "TcpClientRead";

	int out = addParamOut("result");
	setPinType(out, { "QByteArray" });

	m_function = "JZCommTcpRead";
	m_output << irId(out);
}
JZNodeTcpClientRead::~JZNodeTcpClientRead()
{
}

//JZNodeTcpClientWrite
JZNodeTcpClientWrite::JZNodeTcpClientWrite() 
{
	m_type = Node_TcpClientWrite;
	m_name = "TcpClientWrite";

	int in = addParamIn("send");
	setPinType(in, { "QByteArray" });

	m_function = "JZCommTcpClientWrite";
	m_input << irId(in);
}

JZNodeTcpClientWrite::~JZNodeTcpClientWrite() 
{
}

//JZNodeTcpClientRead
JZNodeTcpClientReadText::JZNodeTcpClientReadText()
{
	m_type = Node_TcpClientReadText;
	m_name = "TcpClientReadText";

	int out = addParamOut("result");
	setPinTypeString(out);

	m_function = "JZCommTcpReadText";
	m_output << irId(out);
}
JZNodeTcpClientReadText::~JZNodeTcpClientReadText()
{
}

//JZNodeTcpClientWrite
JZNodeTcpClientWriteText::JZNodeTcpClientWriteText() 
{
	m_type = Node_TcpClientWriteText;
	m_name = "TcpClientWriteText";

	int in = addParamIn("send");
	setPinTypeString(in);

	m_function = "JZCommTcpClientWriteText";
	m_input << irId(in);
}

JZNodeTcpClientWriteText::~JZNodeTcpClientWriteText() 
{
}

//JZNodeUdpRead
JZNodeUdpRead::JZNodeUdpRead() 
{
	m_type = Node_UdpRead;
	m_name = "UdpRead";

	int out = addParamIn("send");
	setPinType(out, { "QByteArray" });

	m_function = "JZCommUdpRead";
	m_output << irId(out);
}
JZNodeUdpRead::~JZNodeUdpRead() 
{
}

//JZNodeUdpWrite
JZNodeUdpWrite::JZNodeUdpWrite()
{
	m_type = Node_UdpWrite;
	m_name = "UdpWrite";

	int in = addParamIn("send");
	setPinType(in, { "QByteArray" });

	m_function = "JZCommUdpWriteText";
	m_input << irId(in);
}
JZNodeUdpWrite::~JZNodeUdpWrite()
{
}


//JZNodeSerialRead
JZNodeSerialRead::JZNodeSerialRead() 
{
	m_type = Node_SerialRead;
	m_name = "SerialRead";

	int out = addParamIn("result");
	setPinType(out, { "QByteArray" });

	m_function = "JZCommSerialRead";
	m_output << irId(out);

}
JZNodeSerialRead::~JZNodeSerialRead()
{
}

//JZNodeSerialWrite
JZNodeSerialWrite::JZNodeSerialWrite() 
{
	m_type = Node_SerialWrite;
	m_name = "SerialWrite";

	int in = addParamIn("result");
	setPinType(in, { "QByteArray" });

	m_function = "JZCommSerialWrite";
	m_input << irId(in);
}
JZNodeSerialWrite::~JZNodeSerialWrite() 
{
}

//JZNodeSerialReadText
JZNodeSerialReadText::JZNodeSerialReadText()
{
	m_type = Node_SerialReadText;
	m_name = "SerialReadText";

	int out = addParamIn("result");
	setPinTypeString(out);

	m_function = "JZCommSerialReadText";
	m_output << irId(out);
}

JZNodeSerialReadText::~JZNodeSerialReadText()
{
}

//JZNodeSerialWriteText
JZNodeSerialWriteText::JZNodeSerialWriteText()
{
	m_type = Node_SerialWriteText;
	m_name = "SerialWriteText";

	int in = addParamOut("send");
	setPinTypeString(in);

	m_function = "JZCommSerialWriteText";
	m_input << irId(in);
}

JZNodeSerialWriteText::~JZNodeSerialWriteText()
{
}
