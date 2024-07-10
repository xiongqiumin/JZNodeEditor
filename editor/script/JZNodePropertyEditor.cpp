#include "JZNodePropertyEditor.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include "JZNodeTypeHelper.h"

JZNodePropertyEditor::JZNodePropertyEditor(QWidget *widget)
    :QWidget(widget)
{
    m_node = nullptr;    
    m_tree = new JZNodePropertyBrowser();
    m_editing = false;

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->addWidget(m_tree);
    setLayout(l);
    
    connect(m_tree,&JZNodePropertyBrowser::valueChanged,this,&JZNodePropertyEditor::onValueChanged);
}

JZNodePropertyEditor::~JZNodePropertyEditor()
{
}

void JZNodePropertyEditor::clear()
{
    m_editing = false;
    m_tree->clear();
    m_tree->clear();
    m_propMap.clear();
    
    m_node = nullptr;    
}

void JZNodePropertyEditor::onValueChanged(JZNodeProperty *p, const QString &value)
{    
    if (m_editing)
        return;
    
    if (m_node)
    {
        int prop_id = m_propMap.key(p, -1);
        if (prop_id != -1)        
            emit sigNodePropChanged(m_node->id(), prop_id, value);
    }
}

JZNode *JZNodePropertyEditor::node()
{
    return m_node;
}

void JZNodePropertyEditor::setPinValue(int prop_id,const QString &value)
{
    if(!m_propMap.contains(prop_id))
        return;

    m_tree->blockSignals(true);
    m_propMap[prop_id]->setValue(value);
    m_tree->blockSignals(false);
}

void JZNodePropertyEditor::setPropEditable(int prop_id,bool editable)
{    
    if(!m_propMap.contains(prop_id))
        return;

    m_propMap[prop_id]->setEnabled(editable);
}

JZNodeProperty *JZNodePropertyEditor::createProp(JZNodePin *pin)
{
    int type = JZNodeType::upType(pin->dataTypeId());
    if (type == Type_none && pin->dataType().size() > 0)
        type = Type_any;

    auto pin_prop = new JZNodeProperty(pin->name(), NodeProprety_GroupId);
    //pin_prop->setDataType(pin->dataType());
    pin_prop->setValue(pin->value());
    if(type == Type_none || !(pin->flag() & Pin_editValue))
        pin_prop->setEnabled(false);
    m_propMap[pin->id()] = pin_prop;
    return pin_prop;
}

void JZNodePropertyEditor::addPropList(QString name,const QList<int> &list)
{
    if(list.size() == 0)
        return;

    JZNodeProperty *prop_group = nullptr;
    for(int i = 0; i < list.size(); i++)
    {
        auto pin = m_node->pin(list[i]);
        if (pin->isWidget())
            continue;

        if(prop_group == nullptr)
            prop_group = new JZNodeProperty(name, NodeProprety_GroupId);
        
        auto pin_prop = createProp(pin);
        prop_group->addSubProperty(pin_prop);        
    }

    if(prop_group != nullptr)
        m_tree->addProperty(prop_group);
}

void JZNodePropertyEditor::setNode(JZNode *node)
{        
    m_node = node;
    updateNode();
}

void JZNodePropertyEditor::updateNode()
{
    auto tmp_node = m_node;
    clear();    
    m_node = tmp_node;
    if(!m_node)
        return;

    m_editing = true;

    auto prop_base = new JZNodeProperty("基本信息", NodeProprety_GroupId);
    auto prop_name = new JZNodeProperty("名称", NodeProprety_Value);
    auto prop_id = new JZNodeProperty("Id", NodeProprety_NodeId);
    prop_base->addSubProperty(prop_name);
    prop_base->addSubProperty(prop_id);
    prop_name->setDataType({ Type_string });
    prop_name->setValue(m_node->name());
    prop_name->setEnabled(false);
    prop_id->setValue(QString::number(m_node->id()));    
    prop_id->setEnabled(false);
    m_tree->addProperty(prop_base);            

    auto in_list = m_node->pinInList(Pin_param);
    addPropList("输入",in_list);
    auto out_list = m_node->pinOutList(Pin_param);
    addPropList("输出",out_list);

    m_editing = false;
}