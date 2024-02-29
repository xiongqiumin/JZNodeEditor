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
    m_tree = new QtTreePropertyBrowser();
    m_editing = false;

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->addWidget(m_tree);
    setLayout(l);

    m_propManager = new QtVariantPropertyManager();
    m_propEditor = new QtVariantEditorFactory();

    connect(m_propManager,&QtVariantPropertyManager::valueChanged,this,&JZNodePropertyEditor::onValueChanged);
    m_tree->setFactoryForManager(m_propManager, m_propEditor);
}

JZNodePropertyEditor::~JZNodePropertyEditor()
{
}

void JZNodePropertyEditor::clear()
{
    m_editing = false;
    m_tree->clear();
    m_propManager->clear();
    m_propMap.clear();
    m_propNameMap.clear();
    
    m_node = nullptr;    
}

void JZNodePropertyEditor::onValueChanged(QtProperty *pin, const QVariant &value)
{    
    if (m_editing)
        return;

    QtVariantProperty *p = dynamic_cast<QtVariantProperty*>(pin);        
    if (m_node)
    {
        int prop_id = m_propMap.key(p, -1);
        if (prop_id != -1)        
            emit sigNodePropChanged(m_node->id(), prop_id, value.toString());

        int prop_name_id = m_propNameMap.key(p, -1);
        if (prop_name_id != -1)        
            emit sigNodePropNameChanged(m_node->id(), prop_name_id, value.toString());        
    }
}

JZNode *JZNodePropertyEditor::node()
{
    return m_node;
}

void JZNodePropertyEditor::setPinName(int prop_id,const QString &name)
{
    if(!m_propNameMap.contains(prop_id))
        return;

    m_propManager->blockSignals(true);
    m_propNameMap[prop_id]->setValue(name);
    m_propManager->blockSignals(false);
}

void JZNodePropertyEditor::setPinValue(int prop_id,const QVariant &value)
{
    if(!m_propMap.contains(prop_id))
        return;

    m_propManager->blockSignals(true);
    m_propMap[prop_id]->setValue(value);
    m_propManager->blockSignals(false);
}

void JZNodePropertyEditor::setPropEditable(int prop_id,bool editable)
{    
    if(!m_propMap.contains(prop_id))
        return;

    m_propMap[prop_id]->setEnabled(editable);
}

QtVariantProperty *JZNodePropertyEditor::createPropName(JZNodePin *pin)
{
    auto prop_name = m_propManager->addProperty(QVariant::String, "name");
    prop_name->setValue(pin->name());
    m_propNameMap[pin->id()] = prop_name;
    return prop_name;
}

QtVariantProperty *JZNodePropertyEditor::createProp(JZNodePin *pin)
{
    QString pin_name = pin->name();
    if(pin->flag() & Pin_editName)
        pin_name = "value";

    auto pin_prop = m_propManager->addProperty(QVariant::String, pin_name);
    pin_prop->setValue(pin->value());
    m_propMap[pin->id()] = pin_prop;
    return pin_prop;
}

void JZNodePropertyEditor::addPropList(QString name,QVector<int> list)
{
    if(list.size() == 0)
        return;

    QtVariantProperty *prop_group = nullptr;
    for(int i = 0; i < list.size(); i++)
    {
        auto pin = m_node->pin(list[i]);
        if(prop_group == nullptr)
            prop_group = m_propManager->addProperty(m_propManager->groupTypeId(),name);
        
        if(pin->flag() & Pin_editName)
        {
            auto prop_name = createPropName(pin);
            prop_group->addSubProperty(prop_name);

            auto prop_value = createProp(pin);
            prop_group->addSubProperty(prop_value);
        }
        else
        {
            auto pin_prop = createProp(pin);
            prop_group->addSubProperty(pin_prop);
        }
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

    auto prop_base = m_propManager->addProperty(m_propManager->groupTypeId(), "基本信息");
    auto prop_name = m_propManager->addProperty(QVariant::String, "名称");
    prop_base->addSubProperty(prop_name);
    prop_name->setValue(m_node->name());
    m_tree->addProperty(prop_base);    

    auto in_list = m_node->pinInList(Pin_param);
    addPropList("输入",in_list);
    auto out_list = m_node->pinOutList(Pin_param);
    addPropList("输出",out_list);

    m_editing = false;
}