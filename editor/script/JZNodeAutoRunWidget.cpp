#include <QVBoxLayout>
#include "JZNodeAutoRunWidget.h"
#include "JZNodeEditor.h"
#include "JZNodeParamDelegate.h"

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

    m_editor = nullptr;
    setDepend(ScriptDepend());
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
    m_depend = ScriptDepend();
    m_tree->clear();
    m_propList.clear();
}

void JZNodeAutoRunWidget::addPin(JZNodeProperty *pin, PinType type, QString name)
{
    PropCoor coor;
    coor.pin = pin;
    coor.type = type;
    coor.name = name;    
    m_propList.push_back(coor);
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

int JZNodeAutoRunWidget::editType(int data_type)
{
    auto inst = JZNodeParamDelegateManager::instance();
    if (inst->hasDelegate(data_type))
        return inst->delegate(data_type)->editType;
    else
        return data_type;
}

void JZNodeAutoRunWidget::copyDependValue(ScriptDepend &old, ScriptDepend &dst)
{    
    auto copyExist = [](QMap<QString,QString> &old, QMap<QString, QString> &dst) {
        auto it = dst.begin();
        while (it != dst.end())
        {
            if (old.contains(it.key()))
            {
                it.value() = old[it.key()];
            }
            it++;
        } 
    };

    if (typeEqual(old.function.paramIn, dst.function.paramIn))
        dst.function.paramIn = old.function.paramIn;
       
    copyExist(old.member, dst.member);
    copyExist(old.global, dst.global);

    for (int i = 0; i < old.hook.size(); i++)    
    {
        auto ptr = dst.getHook(old.hook[i].nodeId);
        if (ptr)
        {
            ptr->enable = old.hook[i].enable;
            ptr->params = old.hook[i].params;            
        }
    }
}

void JZNodeAutoRunWidget::setDepend(const ScriptDepend &depend)
{
    auto obj_inst = m_editor->project()->objectManager();
    auto func_inst = m_editor->project()->functionManager();

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
            sub_item->setDataType(editType(p.dataType()));
            sub_item->setValue(m_depend.function.paramIn[i].value);

            func_input->addSubProperty(sub_item);
            addPin(sub_item, Pin_funcIn, i, -1);
        }
    }

    if (m_depend.member.size() > 0)
    {
        auto item_member = new JZNodeProperty("成员变量", NodeProprety_GroupId);
        item_input->addSubProperty(item_member);

        auto meta = obj_inst->meta(m_depend.function.className);

        auto it = m_depend.member.begin();
        while(it != m_depend.member.end())
        {
            QString name = it.key();
            int data_type = meta->param(name)->dataType();
            QString value = it.value();

            auto sub_item = new JZNodeProperty(name, NodeProprety_Value);
            sub_item->setDataType(editType(data_type));
            sub_item->setValue(value);

            item_member->addSubProperty(sub_item);
            addPin(sub_item, Pin_member, name);

            it++;
        }
    }

    if (m_depend.global.size() > 0)
    {        
        auto item_global = new JZNodeProperty("全局变量", NodeProprety_GroupId);
        item_input->addSubProperty(item_global);

        auto it = m_depend.global.begin();
        while (it != m_depend.global.end())
        {
            QString name = it.key();
            int data_type = m_editor->project()->globalVariable(name)->dataType();
            QString value = it.value();

            auto sub_item = new JZNodeProperty(name, NodeProprety_Value);
            sub_item->setDataType(editType(data_type));
            sub_item->setValue(value);

            item_global->addSubProperty(sub_item);
            addPin(sub_item, Pin_member, name);

            it++;
        }
    }

    if (m_depend.hook.size() > 0)
    {        
        auto item_function_hook = new JZNodeProperty("函数返回", NodeProprety_GroupId);
        item_input->addSubProperty(item_function_hook);

        for (int hook_idx = 0; hook_idx < m_depend.hook.size(); hook_idx++)
        {
            auto &hook = m_depend.hook[hook_idx];
            auto node = m_editor->script()->getNode(hook.nodeId);

            QString id = node->name() + "(" + QString::number(hook.nodeId) + ")";
            auto item_function = new JZNodeProperty(id, NodeProprety_Value);
            item_function_hook->addSubProperty(item_function);                        

            auto enable_item = new JZNodeProperty("hook enable", NodeProprety_Value);
            enable_item->setDataType(Type_hookEnable);
            enable_item->setValue(hook.enable? "true" : "false");
            item_function->addSubProperty(enable_item);
            addPin(enable_item, Pin_hook, 0, hook.nodeId);
            
            auto func = func_inst->function(hook.function);
            auto &node_out = hook.params;
            for (int i = 0; i < node_out.size(); i++)
            {                
                auto sub_item = new JZNodeProperty(func->paramOut[i].name, NodeProprety_Value);
                sub_item->setDataType(editType(func->paramOut[i].dataType()));
                sub_item->setValue(node_out[i]);

                item_function->addSubProperty(sub_item);
                addPin(sub_item, Pin_hook, i+1, hook.nodeId);
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
            sub_item->setDataType(Type_string);
            sub_item->setEnabled(false);

            item_output->addSubProperty(sub_item);       
            addPin(sub_item, Pin_funcOut, i, -1);            
        }
    }
}

const ScriptDepend &JZNodeAutoRunWidget::depend() const
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

    emit sigDependChanged();
}