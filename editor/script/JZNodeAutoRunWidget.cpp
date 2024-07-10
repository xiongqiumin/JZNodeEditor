#include <QVBoxLayout>
#include "JZNodeAutoRunWidget.h"

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

    m_tree = new JZNodePropertyBrowser();
    connect(m_tree, &JZNodePropertyBrowser::valueChanged, this, &JZNodeAutoRunWidget::onValueChanged);

    l->addWidget(m_tree);
    setLayout(l);

    setDepend(ScriptDepend());
}

JZNodeAutoRunWidget::~JZNodeAutoRunWidget()
{
}

void JZNodeAutoRunWidget::clear()
{
    m_depend = ScriptDepend();
    m_tree->clear();
    m_propList.clear();
}

void JZNodeAutoRunWidget::addPin(JZNodeProperty *pin, PinType type, int index, int nodeId)
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

void JZNodeAutoRunWidget::copyDependValue(ScriptDepend &old, ScriptDepend &dst)
{
    if (typeEqual(old.function.paramIn, dst.function.paramIn))
        dst.function.paramIn = old.function.paramIn;

    for (int i = 0; i < old.global.size(); i++)
    {
        for (int j = 0; j < dst.global.size(); j++)
        {
            if (typeEqual(old.global[i], dst.global[j]))
                dst.global[j] = old.global[i];
        }
    }

    for (int i = 0; i < old.hook.size(); i++)    
    {
        auto ptr = dst.getHook(old.hook[i].nodeId);
        if (ptr)
        {
            if (typeEqual(ptr->params, old.hook[i].params))
                ptr->params = old.hook[i].params;
        }
    }
}

void JZNodeAutoRunWidget::setDepend(const ScriptDepend &depend)
{
    auto old = m_depend;
    m_depend = depend;
    copyDependValue(old,m_depend);
    
    m_tree->clear();
    m_propList.clear();
    auto item_input = new JZNodeProperty("输入依赖", NodeProprety_GroupId);    
    m_tree->addProperty(item_input);    

    int param_start = (m_depend.function.isMemberFunction())? 1:0;
    if (m_depend.function.paramIn.size() > param_start)
    {
        auto func_input = new JZNodeProperty("输入参数", NodeProprety_GroupId);
        item_input->addSubProperty(func_input);

        for (int i = param_start; i < m_depend.function.paramIn.size(); i++)
        {
            auto &p = m_depend.function.paramIn[i];
            auto sub_item = new JZNodeProperty(p.name, NodeProprety_Value);
            sub_item->setDataType({ p.dataType()});
            sub_item->setValue(m_depend.function.paramIn[i].value);

            func_input->addSubProperty(sub_item);
            addPin(sub_item, Pin_funcIn, i);
        }
    }

    if (m_depend.member.size() > 0)
    {
        auto item_member = new JZNodeProperty("成员变量", NodeProprety_GroupId);
        item_input->addSubProperty(item_member);

        for (int i = 0; i < m_depend.member.size(); i++)
        {
            auto sub_item = new JZNodeProperty(m_depend.member[i].name, NodeProprety_Value);
            sub_item->setDataType({ m_depend.member[i].dataType()} );
            sub_item->setValue(m_depend.member[i].value);

            item_member->addSubProperty(sub_item);
            addPin(sub_item, Pin_member, i);
        }
    }

    if (m_depend.global.size() > 0)
    {        
        auto item_global = new JZNodeProperty("全局变量", NodeProprety_GroupId);
        item_input->addSubProperty(item_global);

        for (int i = 0; i < m_depend.global.size(); i++)
        {
            auto sub_item = new JZNodeProperty(m_depend.global[i].name, NodeProprety_Value);
            sub_item->setDataType({ m_depend.global[i].dataType() });
            sub_item->setValue(m_depend.global[i].value);

            item_global->addSubProperty(sub_item);
            addPin(sub_item, Pin_global, i);
        }
    }

    if (m_depend.hook.size() > 0)
    {        
        auto item_function_hook = new JZNodeProperty("函数返回", NodeProprety_FunctionHook);        
        item_input->addSubProperty(item_function_hook);

        for (int i = 0; i < m_depend.hook.size(); i++)
        {
            QString id = "(" + QString::number(m_depend.hook[i].nodeId) + ")";
            auto item_function = new JZNodeProperty(id, NodeProprety_GroupId);
            item_function_hook->addSubProperty(item_function);

            auto &node_out = m_depend.hook[i].params;
            for (int i = 0; i < node_out.size(); i++)
            {
                auto sub_item = new JZNodeProperty(node_out[i].name, NodeProprety_Value);
                sub_item->setDataType({ node_out[i].dataType() });
                sub_item->setValue(node_out[i].value);

                item_function->addSubProperty(sub_item);
                addPin(sub_item, Pin_hook, i, m_depend.hook[i].nodeId);
            }
        }
    }
    
    if (m_depend.function.paramOut.size() > 0)
    {        
        auto item_output = new JZNodeProperty("运行输出", NodeProprety_GroupId);
        m_tree->addProperty(item_output);
        
        for (int i = 0; i < m_depend.function.paramOut.size(); i++)
        {
            auto &p = m_depend.function.paramOut[i];
            auto sub_item = new JZNodeProperty(p.name, NodeProprety_Value);
            sub_item->setDataType({ Type_string });
            sub_item->setEnabled(false);

            item_output->addSubProperty(sub_item);       
            addPin(sub_item, Pin_funcOut, i);            
        }
    }
}

const ScriptDepend &JZNodeAutoRunWidget::depend() const
{
    Q_ASSERT(!m_depend.function.isNull());
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

void JZNodeAutoRunWidget::onValueChanged(JZNodeProperty *pin, const QString &value)
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

    if (coor->type == Pin_funcIn)
        m_depend.function.paramIn[coor->index].value = value;
    else if(coor->type == Pin_member)
        m_depend.member[coor->index].value = value;
    else if(coor->type == Pin_global)
        m_depend.global[coor->index].value = value;    
    else if(coor->type == Pin_hook)
        m_depend.hook[coor->nodeId].params[coor->index].value = value;

    emit sigDependChanged();
}