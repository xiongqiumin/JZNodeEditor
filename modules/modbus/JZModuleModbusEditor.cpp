#include "JZModuleModbusEditor.h"

/*
JZNodeModbusConfig::JZNodeModbusConfig()
{
    m_type = Node_modbusConfig;
}

JZNodeModbusConfig::~JZNodeModbusConfig()
{

}

QString JZNodeModbusConfig::className()
{
    return "JZ" + m_functionName.mid(4);
}

void JZNodeModbusConfig::initFunction()
{
    addFlowIn();
    addFlowOut();
        
    QString class_type = className();
    int in = addParamIn("modbus");
    pin(in)->setDataType({class_type});
    
    addParamIn("");
    setName(m_functionName);
}

JZNodePinWidget *JZNodeModbusConfig::createWidget(int id)
{
    if (id == paramIn(1))
    {
        JZNodePinButtonWidget *w = new JZNodePinButtonWidget(this,id);
        auto btn = w->button();
        btn->setText("设置");
        btn->connect(btn, &QPushButton::clicked, [btn,this] {
            QByteArray old = this->toBuffer();

            JZModbusConfigDialog dlg;
            dlg.setConfigMode(true);
            dlg.setConfig(this->m_config);
            if (dlg.exec() != QDialog::Accepted)
                return;

            m_config = dlg.config();
            propertyChangedNotify(old);
        });
        return w;
    }

    return nullptr;
}

bool JZNodeModbusConfig::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;    

    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << m_config;

    int obj_id = c->paramId(m_id, paramIn(0));
    int id = c->allocStack(Type_byteArray);
    c->addSetBuffer(irId(id), buffer);

    QList<JZNodeIRParam> in, out;
    in << irId(obj_id) << irId(id);
    c->addCallVirtual(m_functionName,in,out);
    c->addFlowJump(flowOut());

    return true;
}

void JZNodeModbusConfig::saveToStream(QDataStream &s) const
{
    JZNodeFunctionCustom::saveToStream(s);
    s << m_functionName << m_config;
}

void JZNodeModbusConfig::loadFromStream(QDataStream &s)
{
    JZNodeFunctionCustom::loadFromStream(s);
    s >> m_functionName >> m_config;
}


void initModbusMaster(JZModbusMaster *master, const QByteArray &buffer)
{
    JZModbusConfig config;
    QDataStream s(buffer);
    s >> config;
    modbusMasterSetConfig(master, &config);
}

void initModbusSlaver(JZModbusSlaver *slaver,const QByteArray &buffer)
{
    JZModbusConfig config;
    QDataStream s(buffer);
    s >> config;
    modbusSlaverSetConfig(slaver, &config);
}

JZNodeEditorManager::instance()->registCustomFunctionNode("initModbusMaster", Node_modbusConfig);
JZNodeEditorManager::instance()->registCustomFunctionNode("initModbusSlaver", Node_modbusConfig);
*/