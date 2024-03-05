#include <QVBoxLayout>
#include "JZNodeAutoRunWidget.h"

//PropCoor
JZNodeAutoRunWidget::PropCoor::PropCoor()
{
    pin = nullptr;
    type = -1;
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

void JZNodeAutoRunWidget::addPin(JZNodeProperty *pin, int type, int index, int nodeId)
{
    PropCoor coor;
    coor.pin = pin;
    coor.type = type;
    coor.index = index;
    coor.nodeId = nodeId;;
    m_propList.push_back(coor);
}

void JZNodeAutoRunWidget::setDepend(const ScriptDepend &depend)
{
    clear();
    m_depend = depend;

    auto item_input = new JZNodeProperty("输入依赖", NodeProprety_GroupId);    
    m_tree->addProperty(item_input);    

    int param_start = (depend.function.isMemberFunction())? 1:0;
    if (depend.function.paramIn.size() > param_start)
    {
        auto func_input = new JZNodeProperty("输入参数", NodeProprety_GroupId);
        item_input->addSubProperty(func_input);

        for (int i = param_start; i < depend.function.paramIn.size(); i++)
        {
            auto &p = depend.function.paramIn[i];
            auto sub_item = new JZNodeProperty(p.name, p.dataType());            
            sub_item->setValue(depend.function.paramIn[i].initValue());

            func_input->addSubProperty(sub_item);
            addPin(sub_item, Pin_funcIn, i);
        }
    }

    if (depend.member.size() > 0)
    {
        auto item_member = new JZNodeProperty("成员变量", NodeProprety_GroupId);
        item_input->addSubProperty(item_member);

        for (int i = 0; i < depend.member.size(); i++)
        {
            auto sub_item = new JZNodeProperty(depend.member[i].name, depend.member[i].dataType());
            sub_item->setValue(depend.member[i].initValue());

            item_member->addSubProperty(sub_item);
            addPin(sub_item, Pin_member, i);
        }
    }

    if (depend.global.size() > 0)
    {        
        auto item_global = new JZNodeProperty("全局变量", NodeProprety_GroupId);
        item_input->addSubProperty(item_global);

        for (int i = 0; i < depend.global.size(); i++)
        {
            auto sub_item = new JZNodeProperty(depend.global[i].name, depend.global[i].dataType());
            sub_item->setValue(depend.global[i].initValue());

            item_global->addSubProperty(sub_item);
            addPin(sub_item, Pin_global, i);
        }
    }

    if (depend.hook.size() > 0)
    {        
        auto item_function_hook = new JZNodeProperty("函数返回", NodeProprety_GroupId);
        item_input->addSubProperty(item_function_hook);

        auto it = depend.hook.begin();
        while(it != depend.hook.end())
        {
            QString id = "(" + QString::number(it.key()) + ")";
            auto item_function = new JZNodeProperty(id, NodeProprety_GroupId);
            item_function_hook->addSubProperty(item_function);

            auto &node_out = it.value();
            for (int i = 0; i < node_out.size(); i++)
            {
                auto sub_item = new JZNodeProperty(node_out[i].name, node_out[i].dataType());
                sub_item->setValue(node_out[i].initValue());

                item_function->addSubProperty(sub_item);
                addPin(sub_item, Pin_hook, i, it.key());
            }
            it++;
        }
    }

    auto item_output = new JZNodeProperty("运行输出", NodeProprety_GroupId);
    if (depend.function.paramOut.size() > 0)
    {        
        m_tree->addProperty(item_output);
        
        for (int i = 0; i < depend.function.paramOut.size(); i++)
        {
            auto &p = depend.function.paramOut[i];
            auto sub_item = new JZNodeProperty(p.name, p.dataType());
            sub_item->setValue(depend.function.paramOut[i].value);

            item_output->addSubProperty(sub_item);            
        }
    }
}

const ScriptDepend &JZNodeAutoRunWidget::depend() const
{
    return m_depend;
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
        m_depend.hook[coor->nodeId][coor->index].value = value;

    emit sigDependChanged();
}