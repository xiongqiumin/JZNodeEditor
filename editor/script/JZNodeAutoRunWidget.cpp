#include <QVBoxLayout>
#include "JZNodeAutoRunWidget.h"
#include "JZNodeEditor.h"

//PropCoor
JZNodeAutoRunWidget::PropCoor::PropCoor()
{
    pin = nullptr;
    type = Pin_none;
    nodeId = -1;
    index = -1;
}

//JZNodeAutoRunWidget
JZNodeAutoRunWidget::JZNodeAutoRunWidget(QWidget *p)
    :QWidget(p)
{
    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    m_tree = new JZPropertyBrowser();
    connect(m_tree, &JZPropertyBrowser::valueChanged, this, &JZNodeAutoRunWidget::onValueChanged);

    l->addWidget(m_tree);
    setLayout(l);

    m_editor = nullptr;
    m_depend = nullptr;
}

JZNodeAutoRunWidget::~JZNodeAutoRunWidget()
{
}

void JZNodeAutoRunWidget::setEditor(JZNodeEditor *editor)
{
    m_editor = editor;
}

void JZNodeAutoRunWidget::clear()
{
    m_depend = nullptr;
    m_tree->clear();
    m_propList.clear();
}

void JZNodeAutoRunWidget::addPin(JZProperty *pin, PinType type, QString name)
{
    PropCoor coor;
    coor.pin = pin;
    coor.type = type;
    coor.name = name;    
    m_propList.push_back(coor);
}

void JZNodeAutoRunWidget::addPin(JZProperty *pin, PinType type, int index, int nodeId)
{
    PropCoor coor;
    coor.pin = pin;
    coor.type = type;
    coor.index = index;
    coor.nodeId = nodeId;;
    m_propList.push_back(coor);
}

bool JZNodeAutoRunWidget::typeEqual(const JZParamDefine &p1, const JZParamDefine &p2)
{
    return p1.name == p2.name && p1.type == p2.type;
}

bool JZNodeAutoRunWidget::typeEqual(const QList<JZParamDefine> &p1, const QList<JZParamDefine> &p2)
{
    if (p1.size() != p2.size())
        return false;

    for (int i = 0; i < p1.size(); i++)
    {
        if (!typeEqual(p1[i], p2[i]))
            return false;
    }
    return true;
}

int JZNodeAutoRunWidget::editType(int data_type)
{    
    auto env = editorEnvironment();
    auto inst = JZNodeEditorManager::instance();
    auto d = inst->delegate(data_type);
    if (d && d->editType != Type_none)
        return inst->delegate(data_type)->editType;
    else
        return data_type;
}

void JZNodeAutoRunWidget::setDepend(JZScriptItemDepend *depend)
{
    if (depend == nullptr)
    {
        clear();
        return;
    }
    m_depend = depend;
}

JZScriptItemDepend *JZNodeAutoRunWidget::depend()
{
    return m_depend;
}

void JZNodeAutoRunWidget::setResult(QVariantList params)
{
    m_tree->blockSignals(true);
    for (int i = 0; i < params.size(); i++)
    {
        auto coor = propCoor(Pin_funcOut, i);
        QString text = JZNodeType::debugString(params[i]);
        coor->pin->setValue(text);
    }
    m_tree->blockSignals(false);
}

JZNodeAutoRunWidget::PropCoor *JZNodeAutoRunWidget::propCoor(PinType type, int index)
{
    for (int i = 0; i < m_propList.size(); i++)
    {
        if (m_propList[i].index == index && m_propList[i].type == type)
            return &m_propList[i];
    }
    return nullptr;
}

void JZNodeAutoRunWidget::onValueChanged(JZProperty *pin, const QVariant &value)
{
    PropCoor *coor = nullptr;
    for (int i = 0; i < m_propList.size(); i++)
    {
        if (m_propList[i].pin == pin)
        {
            coor = &m_propList[i];
            break;
        }
    }
    Q_ASSERT(coor);
/*
    if (coor->type == Pin_funcIn)
        m_depend.function.paramIn[coor->index].value = value;
    else if(coor->type == Pin_member)
        m_depend.member[coor->name] = value;
    else if(coor->type == Pin_global)
        m_depend.global[coor->name] = value;
    else if (coor->type == Pin_hook)
    {
        auto ptr = m_depend.getHook(coor->nodeId);
        if (coor->index == 0)
            ptr->enable = (value == "true");   
        else
            ptr->params[coor->index - 1] = value;
    }
*/
    emit sigDependChanged();
}