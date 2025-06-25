#include "JZModuleCommEditor.h"
#include "JZModuleComm.h"
#include "JZEditorGlobal.h"
#include "JZRegExpHelp.h"
#include "JZNodeView.h"
#include "JZCommManager.h"


//JZCommInitDialog
JZCommInitDialog::JZCommInitDialog(QWidget *parent)
    :JZNodeManagerDialog(parent)
{
    QStringList strListHeader = { "名称", "类型" };
    m_table->setColumnCount(strListHeader.size());
    m_table->setHorizontalHeaderLabels(strListHeader);

    m_camTypeList = QStringList{ "None","ModbusRtuClient","ModbusTcpClient" };
}

void JZCommInitDialog::setConfig(JZCommManagerConfig cfg)
{
    m_config = cfg;
    updateConfig();
}

JZCommManagerConfig JZCommInitDialog::config()
{
    return m_config;
}

void JZCommInitDialog::addConfig() 
{
    QStringList camera_list;
    for (int i = 0; i < m_config.commList.size(); i++)
        camera_list << m_config.commList[i]->name;

    JZCommModbusTcpClientConfig *cfg = new JZCommModbusTcpClientConfig();
    cfg->name = JZRegExpHelp::uniqueString("comm", camera_list);    

    JZCommConfigDialog dlg(this);
    dlg.setConfig(JZCommConfigEnum(cfg));
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.commList << dlg.getConfig();
    updateConfig();
}

void JZCommInitDialog::removeConfig(int index) 
{
    m_config.commList.removeAt(index);
    updateConfig();
}

void JZCommInitDialog::settingConfig(int index) 
{
    JZCommConfigDialog dlg(this);
    dlg.setConfig(m_config.commList[index]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.commList[index] = dlg.getConfig();
    updateConfig();
}

void JZCommInitDialog::updateConfig()
{
    m_table->setRowCount(m_config.commList.size());

    QTableWidget *item = new QTableWidget();    
    for (int i = 0; i < m_config.commList.size(); i++)
    {
        auto &cfg = m_config.commList[i];
        QTableWidgetItem *item = new QTableWidgetItem(cfg->name);
        m_table->setItem(i, 0, item);

        QTableWidgetItem *item_type = new QTableWidgetItem(m_camTypeList[cfg->type]);
        m_table->setItem(i, 1, item_type);
    }
}

//JZCommInitItem    
JZCommInitItem::JZCommInitItem(JZNode *node)
    :JZNodeGraphItem(node)
{
    QPushButton *btnSet = new QPushButton("Setting");
    btnSet->connect(btnSet, &QPushButton::clicked, [this] {
        this->onSetClicked();
    });
    m_setting = createWidgetBlock(btnSet, true);
    m_setting->pri = 8;
}

void JZCommInitItem::onSetClicked()
{
    JZNodeCommInit *node = (JZNodeCommInit *)m_node;
    JZCommInitDialog dlg(editor());
    dlg.setConfig(node->config());
    if (dlg.exec() != QDialog::Accepted)
        return;

    QByteArray oldValue = saveNode();
    node->setConfig(dlg.config());
    QByteArray newValue = saveNode();
    if (newValue == oldValue)
        return;

    notifyPropChanged(oldValue);
}

//JZCommModbusRWItem
JZCommModbusRWItem::JZCommModbusRWItem(JZNode *node)
    :JZNodeGraphItem(node)
{    
    m_funcList = QStringList{ "Bit", "InputBit", "InputRegister", "Register" };    

    m_modbusFunc = createEditBlock("Func", JZParamEditInfo::createByEnum(m_funcList));
    m_modbusDataType = createEditBlock("Type", JZParamEditInfo()); 
}

void JZCommModbusRWItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    auto in_list = m_node->paramInList();
    m_blocks[in_list[0]]->pri = Pri_user;
    m_modbusFunc->pri = Pri_user + 1;
    m_modbusDataType->pri = Pri_user + 2;
    for(int i = 1; i < in_list.size(); i++)
        m_blocks[in_list[i]]->pri = Pri_user + i + 3;
    
    auto node = dynamic_cast<JZNodeModbusRW*>(m_node);
    if (node->function() == Function_Bit || node->function() == Function_InputBit)
    {
        m_modbusDataType->isEditable = false;        
    }
    else
    {
        QList<int> type_list = { Type_int16, Type_uint16, Type_int, Type_uint, Type_float, Type_double };
        m_dataTypeList = editorEnvironment()->typeListToNameList(type_list);
        m_modbusDataType->isEditable = true;
        m_modbusDataType->edit = JZParamEditInfo::createByEnum(m_dataTypeList);
    }
}

void JZCommModbusRWItem::setBlockValue(int pin, QString value)
{
    QByteArray buffer = saveNode();

    auto node = dynamic_cast<JZNodeModbusRW*>(m_node);
    if (pin == m_modbusFunc->id)
        node->setFunction(m_funcList.indexOf(value));
    else if (pin == m_modbusDataType->id)
        node->setDataType(value);
    else {
        Q_ASSERT(0);        
    }
    notifyPropChanged(buffer);
}

QString JZCommModbusRWItem::blockValue(int pin)
{    
    auto node = dynamic_cast<JZNodeModbusRW*>(m_node);
    if (pin == m_modbusFunc->id)
        return m_funcList[node->function()];
    else if (pin == m_modbusDataType->id)
        return node->dataType();
    else {
        Q_ASSERT(0);
        return QString();
    }        
}

//JZModuleCommEditorInit
void JZModuleCommEditorInit()
{
    auto inst = editorManager()->instance();

    inst->registLogicNode(Node_CommInit,"通信", QString(), CreateJZNodeGraphItem<JZCommInitItem>);
    inst->registLogicNode(Node_ModbusRead,"通信", QString(), CreateJZNodeGraphItem<JZCommModbusRWItem>);
    inst->registLogicNode(Node_ModbusWrite,"通信", QString(), CreateJZNodeGraphItem<JZCommModbusRWItem>);
    inst->registLogicNode(Node_TcpClientRead,"通信");
    inst->registLogicNode(Node_TcpClientWrite,"通信");
    inst->registLogicNode(Node_UdpRead,"通信");
    inst->registLogicNode(Node_UdpWrite,"通信");
    inst->registLogicNode(Node_SerialRead,"通信");
    inst->registLogicNode(Node_SerialWrite,"通信");
}