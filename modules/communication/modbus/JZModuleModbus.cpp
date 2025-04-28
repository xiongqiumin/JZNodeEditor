#include <QSerialPort>
#include <QPushButton>
#include "JZModuleModbus.h"
#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"
#include "JZNodeFactory.h"
#include "3rd/JZCommon/jzModbus/JZModbusMaster.h"
#include "3rd/JZCommon/jzModbus/JZModbusSlaver.h"
#include "JZModbusSimulator.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"
#include "JZNodeEditorManager.h"
#include "../JZModuleComm.h"

JZNodeModbusWatchEvent::JZNodeModbusWatchEvent()
{
    m_type = Node_modbusWatch;
    m_name = "modbusWatch";
}

JZNodeModbusWatchEvent::~JZNodeModbusWatchEvent()
{
}

bool JZNodeModbusWatchEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return true;
}

void JZNodeModbusWatchEvent::saveToStream(QDataStream& s) const
{
    JZNodeSignalEvent::saveToStream(s);
}

void JZNodeModbusWatchEvent::loadFromStream(QDataStream& s)
{
    JZNodeSignalEvent::loadFromStream(s);
}

QList<JZParamDefine> JZNodeModbusWatchEvent::functionParamOut()
{
    QList<JZParamDefine> list;
    return list;
}

void JZNodeModbusWatchEventInit()//QObject *qobj,QObject *modbus,const QByteArray &buffer)
{
/*
    QJsonObject obj = JZNodeJson::formBuffer(buffer);
    int id = obj["param"].toInt();
    QString param = obj["function"].toString();

    if (modbus->inherits("JZModbusSlaver"))
    {
        JZModbusSlaver *slaver = dynamic_cast<JZModbusSlaver*>(modbus);
        slaver->connect(slaver, &JZModbusSlaver::sigParamChanged, qobj, [qobj] {

        });
    }
    else if (modbus->inherits("JZModbusMaster"))
    {
        JZModbusMaster* master = dynamic_cast<JZModbusMaster*>(modbus);
    }
*/
}