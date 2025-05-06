#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include "JZNodeTypeHelper.h"
#include "JZNodePropertyEditor.h"
#include "JZNodeGraphItem.h"
#include "JZNodeView.h"
#include "JZNodeObjectParser.h"

JZNodePropertyEditor::JZNodePropertyEditor(QWidget *widget)
    :QWidget(widget)
{
    m_node = nullptr;    
    m_tree = new JZPropertyBrowser();
    m_editing = false;
    m_view = nullptr;

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->addWidget(m_tree);
    setLayout(l);
    
    connect(m_tree,&JZPropertyBrowser::valueChanged,this,&JZNodePropertyEditor::onValueChanged);
}

JZNodePropertyEditor::~JZNodePropertyEditor()
{
}

void JZNodePropertyEditor::initTransMap()
{
    PropTrans pt_trans;
    pt_trans.creator = [](QString name)->JZPropertyPoint*{
        return new JZPropertyPoint(name,qMetaTypeId<QPoint>());
    };
    pt_trans.toString = [](const QVariant &v){
        auto pt = v.toPoint();
        return QString::asprintf("%d,%d",pt.x(),pt.y());
    };
    pt_trans.fromString = [](const QString &text){
        JZNodeObjectParser parser;
        QVariantList list;
        if(!parser.parseVariantList("i,i",text,list)){
            return QPoint();
        }
        return QPoint(list[0].toInt(),list[1].toInt());
    };

    m_transMap["QPoint"] = pt_trans;
}

void JZNodePropertyEditor::clear()
{
    m_editing = false;
    m_tree->clear();
    m_propMap.clear();
    m_propTrans.clear();
    
    m_node = nullptr;    
    m_item = nullptr;
}

void JZNodePropertyEditor::setView(JZNodeView  *view)
{
    m_view = view;
}

void JZNodePropertyEditor::onValueChanged(JZProperty *p, const QVariant &value)
{    
    if (m_editing)
        return;
    
    if (m_node)
    {
        int prop_id = m_propMap.key(p, -1);
        if (prop_id != -1)        
            emit sigNodePropChanged(m_node->id(), prop_id, propValue(p));
    }
}

JZNode *JZNodePropertyEditor::node()
{
    return m_node;
}

void JZNodePropertyEditor::setPinName(int prop_id,const QString &name)
{
    if(!m_propMap.contains(prop_id))
        return;

    m_tree->blockSignals(true);
    m_propMap[prop_id]->setName(name);
    m_tree->blockSignals(false);
}

void JZNodePropertyEditor::setPinValue(int prop_id,const QString &value)
{
    if(!m_propMap.contains(prop_id))
        return;

    m_tree->blockSignals(true);
    setPropValue(m_propMap[prop_id],value);
    m_tree->blockSignals(false);
}

void JZNodePropertyEditor::setPropEditable(int prop_id,bool editable)
{    
    if(!m_propMap.contains(prop_id))
        return;

    m_propMap[prop_id]->setEnabled(editable);
}

JZProperty *JZNodePropertyEditor::createPropValue(JZNodeGraphItem::Block *block)
{
    const JZParamEditInfo &edit = block->edit;

    auto env = editorEnvironment();    
    JZProperty *pin_prop = nullptr;
    if(edit.type == JZParamEditInfo::Edit_enum)
        pin_prop = new JZPropertyStringEnum(block->name, edit.enumList);
    else if(edit.type == JZParamEditInfo::Edit_file)
        pin_prop = new JZPropertyFilePath(block->name, edit.fileFilter);
    else
    {
        QString pin_type;
        if(block->id < MAX_PIN_ID)
        {
            auto pin = m_node->pin(block->id);
            if(pin->dataType().size() == 1) 
                pin_type = pin->dataType()[0];
        }
        
        if(m_transMap.contains(pin_type))
        {
            pin_prop = m_transMap[pin_type].creator(block->name);
            m_propTrans[pin_prop] = pin_type;
        }
        else
            pin_prop = new JZProperty(block->name,QVariant::String);
    }

    m_propMap[block->id] = pin_prop;
    setPinValue(block->id, m_item->pinValue(block->id));
    return pin_prop;
}

void JZNodePropertyEditor::setPropValue(JZProperty *pin,const QString &value)
{
    if(m_propTrans.contains(pin))
    {
        QString pin_type = m_propTrans[pin];
        pin->setValue(m_transMap[pin_type].fromString(value));
    }
    else
        pin->setValue(value);
}

QString JZNodePropertyEditor::propValue(JZProperty *pin)
{
    if(m_propTrans.contains(pin))
    {
        QString pin_type = m_propTrans[pin];
        return m_transMap[pin_type].toString(pin->value().toString());
    }
    else
        return pin->value().toString();
}

void JZNodePropertyEditor::addPropList(QString name,const QList<int> &list)
{    
    JZProperty *prop_group = nullptr;
    for(int i = 0; i < list.size(); i++)
    {
        int pin_id = list[i];
        auto block = m_item->block(list[i]);
        if(pin_id < MAX_PIN_ID || block->isEditable)
        {
            if(prop_group == nullptr)
                prop_group = new JZPropertyGroup(name);
            
            auto pin_prop = createPropValue(block);
            prop_group->addSubProperty(pin_prop);
            if(!block->isEditable)
                pin_prop->setEnabled(false);
        }
    }

    if(prop_group != nullptr)
        m_tree->addProperty(prop_group);
}

void JZNodePropertyEditor::setNode(JZNode *node)
{        
    m_node = node;
    if(m_node == nullptr)
        m_item = nullptr;
    else
        m_item = m_view->getNodeItem(m_node->id());
    
    updateNode();
}

void JZNodePropertyEditor::updateNode()
{
    if (!m_node)
        return;

    m_tree->clear();
    m_propMap.clear();
    m_propTrans.clear();

    m_editing = true;

    auto prop_base = new JZPropertyGroup("基本信息");
    auto prop_name = new JZProperty("名称", QVariant::String);
    auto prop_id = new JZProperty("Id", QVariant::Int);
    prop_name->setEditable(false);
    prop_id->setEditable(false);

    prop_base->addSubProperty(prop_name);
    prop_base->addSubProperty(prop_id);    
    prop_name->setValue(m_node->name());
    prop_name->setEnabled(false);
    prop_id->setValue(m_node->id());    
    prop_id->setEnabled(false);
    m_tree->addProperty(prop_base);

    auto in_list = m_item->blockList(true);
    addPropList("输入",in_list);
    auto out_list = m_item->blockList(false);
    addPropList("输出",out_list);

    m_editing = false;
}