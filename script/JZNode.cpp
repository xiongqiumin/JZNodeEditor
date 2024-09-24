#include <QComboBox>
#include <QPushButton>
#include "JZNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"
#include "JZNodePinWidget.h"

// JZNodeGemo
int JZNodeGemo::paramId(int nodeId,int pinId)
{
    Q_ASSERT(nodeId >= 0 && pinId >= 0);
    return nodeId * 100 + pinId;
}

JZNodeGemo JZNodeGemo::paramGemo(int id)
{
    if (id < Stack_User)
        return JZNodeGemo(id / 100, id % 100);
    else
        return JZNodeGemo();
}

JZNodeGemo::JZNodeGemo()
{
    nodeId = -1;
    pinId = -1;
}

JZNodeGemo::JZNodeGemo(int id, int pin)
{
    nodeId = id;
    pinId = pin;
}

bool JZNodeGemo::isNull() const
{
    return (nodeId == -1);
}

bool JZNodeGemo::operator==(const JZNodeGemo &other) const
{
    return nodeId == other.nodeId && pinId == other.pinId;
}

// JZNodeConnect
JZNodeConnect::JZNodeConnect()
{
    id = -1;
}

void operator<<(QDataStream &s, const JZNodeConnect &param)
{
    s << param.id;
    s << param.from.nodeId;
    s << param.from.pinId;
    s << param.to.nodeId;
    s << param.to.pinId;
}

void operator>>(QDataStream &s, JZNodeConnect &param)
{
    s >> param.id;
    s >> param.from.nodeId;
    s >> param.from.pinId;
    s >> param.to.nodeId;
    s >> param.to.pinId;
}

//JZNodeGroup
JZNodeGroup::JZNodeGroup()
{
    id = -1;
}

void operator<<(QDataStream &s, const JZNodeGroup &param)
{
    s << param.id;
    s << param.memo;
}

void operator>>(QDataStream &s, JZNodeGroup &param)
{
    s >> param.id;
    s >> param.memo;
}

// JZNode
JZNode::JZNode()
{
    m_id = INVALID_ID;
    m_file = nullptr;
    m_flag = NodeProp_none;
    m_type = Node_none;
    m_group = -1;
}

JZNode::~JZNode()
{
}

QByteArray JZNode::toBuffer()
{
    Q_ASSERT(m_type != Node_none);

    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    saveToStream(s);
    return buffer;
}

bool JZNode::fromBuffer(const QByteArray &buffer)
{
    QDataStream s(buffer);    
    loadFromStream(s);
    return true;
}

int JZNode::type() const
{
    return m_type;
}

void JZNode::setFlag(int flag)
{
    m_flag = flag;
}

int JZNode::flag() const
{
    return m_flag;
}

void JZNode::setGroup(int group)
{
    m_group = group;
}

int JZNode::group()
{
    return m_group;
}

const QString &JZNode::memo() const
{
    return m_memo;
}

void JZNode::setMemo(const QString &text)
{
    m_memo = text;
}

bool JZNode::isFlowNode() const
{
    return pinCount(Pin_flow) > 0;
}

bool JZNode::isParamNode() const
{
    return pinCount(Pin_flow) == 0;
}

int JZNode::addPin(const JZNodePin &pin)
{
    Q_ASSERT(pin.isInput() || pin.isOutput());
    Q_ASSERT(pin.isFlow() || pin.isParam() || pin.isSubFlow() || pin.isWidget());

    int max_id = 0;
    for (int i = 0; i < m_pinList.size(); i++)
        max_id = qMax(max_id, m_pinList[i].id() + 1);

    JZNodePin new_prop = pin;
    new_prop.setId(max_id);
    m_pinList.push_back(new_prop);
    return max_id;
}

void JZNode::clearPin()
{
    m_pinList.clear();
}

int JZNode::addParamIn(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Pin_in | Pin_param | extFlag);
    return addPin(pin);        
}

int JZNode::addParamOut(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Pin_out | Pin_param | extFlag);
    return addPin(pin);
}

int JZNode::addFlowIn(int extFlag)
{
    JZNodePin pin;
    pin.setFlag(Pin_in | Pin_flow | extFlag);
    return addPin(pin);
}

int JZNode::addFlowOut(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Pin_out | Pin_flow | extFlag);
    return addPin(pin);
}

int JZNode::addSubFlowOut(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Pin_out | Pin_subFlow | extFlag);
    return addPin(pin);
}

void JZNode::removePin(int id)
{
    int index = indexOfPin(id);
    if (index == -1)
        return;

    m_pinList.removeAt(index);
}

JZNodePin *JZNode::pin(int id)
{
    int index = indexOfPin(id);
    if (index >= 0)
        return &m_pinList[index];
    else
        return nullptr;
}

const JZNodePin *JZNode::pin(int id) const
{
    int index = indexOfPin(id);
    if (index >= 0)
        return &m_pinList[index];
    else
        return nullptr;
}

JZNodePin *JZNode::pin(QString name)
{
    int index = indexOfPinByName(name);
    if (index >= 0)
        return &m_pinList[index];
    else
        return nullptr;
}

bool JZNode::hasPin(int id) const
{
    return indexOfPin(id) >= 0;
}

int JZNode::indexOfPin(int id) const
{
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if (m_pinList[i].id() == id)
            return i;
    }
    return -1;
}

int JZNode::indexOfPinByName(QString name) const
{
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if (m_pinList[i].name() == name)
            return i;
    }
    return -1;
}

int JZNode::indexOfPinByType(int id, int flag) const
{
    int type_index = 0;
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if (m_pinList[i].id() == id)
            return type_index;
        if (m_pinList[i].flag() & flag)
            type_index++;
    }
    return -1;
}

QList<int> JZNode::pinInList(int flag) const
{
    return pinListByType(Pin_in | flag);
}

QList<int> JZNode::pinOutList(int flag) const
{
    return pinListByType(Pin_out | flag);
}

QList<int> JZNode::pinListByType(int flag) const
{
    QList<int> ret;
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if((m_pinList[i].flag() & flag) == flag)
            ret.push_back(m_pinList[i].id());
    }
    return ret;
}

int JZNode::pinCount(int flag) const
{
    int count = 0;
    for (int i = 0; i < m_pinList.size(); i++)
    {
        if ((m_pinList[i].flag() & flag) == flag)
            count++;
    }
    return count;
}

int JZNode::pinPri(int id) const
{
    auto ptr = pin(id);
    if (ptr->isSubFlow())
        return Pri_sub_flow;
    else if (ptr->isFlow())
        return Pri_flow;
    else if (ptr->isParam())
        return Pri_param;
    else if(ptr->isWidget())
        return Pri_widget;
    else
        return Pri_none;        
}

int JZNode::paramIn(int index) const
{
    auto list = pinInList(Pin_param);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::paramInGemo(int index) const
{
    return JZNodeGemo(m_id,paramIn(index));
}

int JZNode::paramInCount() const
{
    return pinInList(Pin_param).size();
}

QList<int> JZNode::paramInList() const
{
    return pinInList(Pin_param);
}

QString JZNode::paramInValue(int index) const
{
    auto list = paramInList();
    return pinValue(list[index]);
}

void JZNode::setParamInValue(int index, const QString &value)
{
    auto list = paramInList();    
    setPinValue(list[index], value);
}

int JZNode::paramOut(int index) const
{
    auto list = pinOutList(Pin_param);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::paramOutGemo(int index) const
{
    return JZNodeGemo(m_id,paramOut(index));
}

int JZNode::paramOutCount() const
{
    return pinOutList(Pin_param).size();
}

QList<int> JZNode::paramOutList() const
{
    return pinOutList(Pin_param);
}

QString JZNode::paramOutValue(int index) const
{
    auto list = paramOutList();
    return pinValue(list[index]);
}

void JZNode::setParamOutValue(int index, const QString &value)
{
    auto list = paramOutList();
    setPinValue(list[index], value);
}

int JZNode::flowIn() const
{
    auto list = pinInList(Pin_flow);
    if(list.size() != 0)
        return list[0];
    else
        return -1;
}

JZNodeGemo JZNode::flowInGemo() const
{
    return JZNodeGemo(m_id,flowIn());
}

int JZNode::flowInCount() const
{
    return pinInList(Pin_flow).size();
}

int JZNode::flowOut(int index) const
{
    auto list = pinOutList(Pin_flow);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::flowOutGemo(int index) const
{
    return JZNodeGemo(m_id,flowOut(index));
}

QList<int> JZNode::flowOutList() const
{
    return pinOutList(Pin_flow);
}

int JZNode::flowOutCount() const
{
    return pinOutList(Pin_flow).size();
}

int JZNode::subFlowOut(int index) const
{
    auto list = pinOutList(Pin_subFlow);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::subFlowOutGemo(int index) const
{
    return JZNodeGemo(m_id,subFlowOut(index));
}

QList<int> JZNode::subFlowList() const
{
    return pinOutList(Pin_subFlow);
}

int JZNode::subFlowCount() const
{
    return pinOutList(Pin_subFlow).size();
}

int JZNode::addWidgetIn(QString name)
{
    JZNodePin btn;
    btn.setName(name);
    btn.setFlag(Pin_widget | Pin_in);
    return addPin(btn);    
}

int JZNode::addWidgetOut(QString name)
{
    JZNodePin btn;
    btn.setName(name);
    btn.setFlag(Pin_widget | Pin_out);
    return addPin(btn);
}

int JZNode::widgetIn(int index) const
{
    auto list = pinInList(Pin_widget);
    if (index < list.size())
        return list[index];
    else
        return -1;
}

int JZNode::widgetOut(int index) const
{
    auto list = pinOutList(Pin_widget);
    if (index < list.size())
        return list[index];
    else
        return -1;
}

const QString &JZNode::pinValue(int id) const
{
    auto ptr = pin(id);
    Q_ASSERT(ptr);

    return ptr->value();
}

void JZNode::setPinValue(int id,const QString &value)
{        
    auto ptr = pin(id);
    Q_ASSERT(ptr);
    
    ptr->setValue(value);
    if(m_file)
        onPinChanged(id);
    else
        m_notifyList << id;
}

const QString &JZNode::pinName(int id) const
{
    auto ptr = pin(id);
    Q_ASSERT(ptr);

    return ptr->name();
}

void JZNode::setPinName(int id,const QString &name)
{
    auto ptr = pin(id);
    Q_ASSERT(ptr);

    ptr->setName(name);
    if(m_file)
        onPinChanged(id);
    else
        m_notifyList << id;
}

QList<int> JZNode::pinList() const
{
    QList<int> result;
    for(int i = 0; i < m_pinList.size(); i++)
        result.push_back(m_pinList[i].id());

    return result;
}

JZScriptItem *JZNode::file() const
{
    return m_file;
}

void JZNode::setFile(JZScriptItem *file)
{
    m_file = file;

    QString error;
    update(error);

    //delay notify
    for(int i = 0; i < m_notifyList.size(); i++)
        onPinChanged(m_notifyList[i]);
    m_notifyList.clear();
}

const JZScriptEnvironment *JZNode::environment() const
{
    if(m_file && m_file->project())
        return m_file->project()->environment();
    
    return nullptr;
}

const QString& JZNode::name() const
{
    return m_name;
}

void JZNode::setName(const QString &name)
{
    m_name = name;
}

QString JZNode::idName() const
{
    return m_name + "(" + QString::number(m_id) + ")";
}

bool JZNode::canRemove()
{
    return !(m_flag & NodeProp_noRemove);
}

bool JZNode::canDragVariable()
{
    return (m_flag & NodeProp_dragVariable);
}

int JZNode::id() const
{
    return m_id;
}

void JZNode::setId(int id)
{
    m_id = id;
}

const QStringList &JZNode::pinType(int id) const
{
    return pin(id)->dataType();
}

void JZNode::setPinTypeArg(int id)
{
    pin(id)->setDataType({ JZNodeType::typeName(Type_arg)});
}

void JZNode::setPinTypeInt(int id)
{
    pin(id)->setDataType({JZNodeType::typeName(Type_int)});
}

void JZNode::setPinTypeNumber(int id)
{
    pin(id)->setDataType({ JZNodeType::typeName(Type_bool),JZNodeType::typeName(Type_int),JZNodeType::typeName(Type_int64),JZNodeType::typeName(Type_double)});
}

void JZNode::setPinTypeBool(int id)
{
    pin(id)->setDataType({ JZNodeType::typeName(Type_bool)});
}

void JZNode::setPinTypeString(int id)
{
    pin(id)->setDataType( { JZNodeType::typeName(Type_string)});
}

void JZNode::setPinType(int id,const QStringList &type)
{
    pin(id)->setDataType(type);
}

void JZNode::clearPinType(int id)
{
    QStringList type;
    pin(id)->setDataType(type);
}

void JZNode::setPinEditType(int id, int edit_type)
{
    pin(id)->setEditType(edit_type);
}

void JZNode::drag(const QVariant &v)
{
    Q_UNUSED(v);
}

bool JZNode::canLink(int node_id, int pin_id, QString &error)
{
    Q_UNUSED(error);
    return true;
}

JZNodePinWidget *JZNode::createWidget(int id)
{
    return nullptr;
}

QStringList JZNode::actionList()
{   
    return QStringList();    
}

bool JZNode::actionTriggered(int)
{
    return false;
}

QStringList JZNode::pinActionList(int)
{
    return QStringList();
}

bool JZNode::pinActionTriggered(int, int)
{
    return false;
}

bool JZNode::update(QString &error)
{
    return true;
}

void JZNode::onPinLinked(int id)
{    
}

void JZNode::onPinUnlinked(int id)
{    
}

void JZNode::onPinChanged(int id)
{
    QString error;
    update(error);
}

void JZNode::propertyChangedNotify(const QByteArray &old)
{
    if (m_file && m_file->project())
        m_file->project()->sigScriptNodeChanged(m_file,m_id, old);
}

void JZNode::widgetChangedNotify(int pin_id)
{
    if (m_file && m_file->project())
        m_file->project()->sigScriptNodeWidgetChanged(m_file,m_id,pin_id);
}

void JZNode::saveToStream(QDataStream &s) const
{
    s << m_type;
    s << m_id;
    s << m_name;
    s << m_flag;
    s << m_group;
    s << m_memo;
    s << m_pinList;
    s << m_notifyList;
}

void JZNode::loadFromStream(QDataStream &s)
{    
    int node_type;
    s >> node_type;
    Q_ASSERT(node_type == m_type);

    s >> m_id;
    s >> m_name;
    s >> m_flag;
    s >> m_group;
    s >> m_memo;
    s >> m_pinList;
    s >> m_notifyList;    
}

//JZNodeNop
JZNodeNop::JZNodeNop()
{
    m_name = "nop";
    m_type = Node_nop;
    addFlowIn();
    addFlowOut();
}
   
bool JZNodeNop::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeDebug(m_id);
    c->addNop();
    c->addJumpNode(flowOut());
    return true;
}


//JZNodeContinue
JZNodeContinue::JZNodeContinue()
{
    m_name = "continue";
    m_type = Node_continue;
    addFlowIn(); 
}

bool JZNodeContinue::compiler(JZNodeCompiler *c,QString &error)
{   
    QVector<int> allow_node = { Node_for,Node_while,Node_foreach };
    auto file = c->currentFile();
    int paremt_id = file->parentNode(m_id);
    if (paremt_id == -1 || !allow_node.contains(file->getNode(paremt_id)->type()))
    {
        error = "无效的break";
        return false;
    }
    c->addContinue();
    return true;
}

//JZNodeBreak
JZNodeBreak::JZNodeBreak()
{
    m_name = "break";
    m_type = Node_break;
    addFlowIn();
}

bool JZNodeBreak::compiler(JZNodeCompiler *c,QString &error)
{       
    QVector<int> allow_node = { Node_for,Node_while,Node_foreach,Node_switch };
    auto file = c->currentFile();
    int paremt_id = file->parentNode(m_id);
    if (paremt_id == -1 || !allow_node.contains(file->getNode(paremt_id)->type()))
    {
        error = "无效的break";
        return false;
    }    
    c->addNodeDebug(m_id);
    c->addBreak();
    return true;
}


//JZNodeReturn
JZNodeReturn::JZNodeReturn()
{        
    m_name = "return";
    m_type = Node_return;
    addFlowIn();
}

bool JZNodeReturn::update(QString &error)
{
    auto env = environment();
    auto inList = paramInList();
    QList<JZParamDefine> ret_list;
    if(m_file->itemType() == ProjectItem_scriptFunction)
        ret_list = m_file->function().paramOut;

    if(inList.size() != ret_list.size())
    {
        error = "function not return " + QString::number(inList.size()) + " param";
        return false;
    }

    auto def = m_file->function();
    for (int i = 0; i < ret_list.size(); i++)
    {
        int in = inList[i];
        setPinType(in, { ret_list[i].type });
    }
    return true;
}

void JZNodeReturn::setFunction(const JZFunctionDefine *def)
{
    auto env = environment();
    auto inList = paramInList();
    for (int i = 0; i < inList.size(); i++)
        removePin(inList[i]);

    for (int i = 0; i < def->paramOut.size(); i++)
    {
        int in = addParamIn(def->paramOut[i].name, Pin_dispName | Pin_dispValue | Pin_editValue);
        setPinType(in, { def->paramOut[i].type });
    }
}

bool JZNodeReturn::compiler(JZNodeCompiler *c,QString &error)
{   
    if(!c->addFlowInput(m_id,error))
        return false;     
    
    auto def = m_file->function();
    c->setRegCallFunction(&def);
    auto inList = paramInList();
    for(int i = 0; i < inList.size(); i++)
    {
        int id = c->paramId(m_id,inList[i]);
        c->addSetVariable(irId(Reg_CallOut + i),irId(id));
    }
    c->setRegCallFunction(nullptr);
    
    JZNodeIR *ir_return = new JZNodeIR(OP_return);    
    c->addStatement(JZNodeIRPtr(ir_return));
    return true;
}

//JZNodeExit
JZNodeExit::JZNodeExit()
{    
    m_name = "exit";
    m_type = Node_exit;
    addFlowIn();
}

bool JZNodeExit::compiler(JZNodeCompiler *c,QString &error)
{
    c->addNodeDebug(m_id);
    JZNodeIR *ir_exit = new JZNodeIR(Node_exit);
    c->addStatement(JZNodeIRPtr(ir_exit));
    return true;
}

//JZNodeSequence
JZNodeSequence::JZNodeSequence()
{
    m_name = "sequence";
    m_type = Node_sequence;
    addFlowIn();
    addFlowOut("complete",Pin_dispName);

    addSequeue();
    addSequeue();

    addWidgetOut("Add pin");    
}

void JZNodeSequence::updateSeqName()
{
    auto list = subFlowList();
    for(int i = 0; i < list.size(); i++)
    {
        QString name = "Seqeue " + QString::number(i + 1);
        pin(list[i])->setName(name);
    }
}

int JZNodeSequence::addSequeue()
{
    return addSubFlowOut("Seqeue " + QString::number(subFlowCount() + 1),Pin_dispName);
}

void JZNodeSequence::removeSequeue(int id)
{
    removePin(id);
    updateSeqName();
}

QStringList JZNodeSequence::pinActionList(int id)
{
    int sub_index = subFlowList().indexOf(id);
    if (sub_index == -1)
        return QStringList();
    if (subFlowCount() < 2)
        return QStringList();
    
    QStringList ret;
    ret.push_back("删除");
    return ret;
}

bool JZNodeSequence::pinActionTriggered(int id, int index)
{
    int pin_index = subFlowList().indexOf(id);
    if (pin_index == -1)
        return false;

    removeSequeue(id);
    return true;
}

bool JZNodeSequence::compiler(JZNodeCompiler *c,QString &error)
{
    QList<int> breakList;
    QList<int> continueList;

    c->addNodeDebug(m_id);
    //设置continue, 最后一个是跳出
    auto list = subFlowList();
    for(int i = 0; i < list.size(); i++)
    {
        int pc = c->addJumpSubNode(list[i]);
        continueList << pc;
        breakList << -1;
    }
    continueList.pop_front();
    continueList << c->addJumpNode(flowOut());

    c->setBreakContinue({breakList},{continueList});
    return true;
}

JZNodePinWidget *JZNodeSequence::createWidget(int id)
{
    JZNodePinButtonWidget *w = new JZNodePinButtonWidget();
    QPushButton *btn = w->button();
    btn->setText("Add Input");
    btn->connect(btn, &QPushButton::clicked, [this] {
        QByteArray old = toBuffer();
        addSequeue();
        propertyChangedNotify(old);
    });        
    return w;
}

// JZNodeParallel
JZNodeParallel::JZNodeParallel()
{
}

//JZNodeFor
JZNodeFor::JZNodeFor()
{
    m_name = "for";
    m_type = Node_for;

    addFlowIn();
    addSubFlowOut("loop body",Pin_dispName);
    addFlowOut("complete",Pin_dispName);

    int id_start = addParamIn("Index",Pin_editValue | Pin_dispName | Pin_dispValue);
    int id_step = addParamIn("Step", Pin_editValue | Pin_dispName | Pin_dispValue);
    int id_end = addParamIn("End index",Pin_editValue | Pin_dispName | Pin_dispValue);
    int id_op = addParamIn("Cond", Pin_dispName | Pin_widget | Pin_noValue);
    int id_index = addParamOut("Index", Pin_dispName);
    setPinTypeInt(id_start);
    setPinTypeInt(id_step);
    setPinTypeInt(id_index);
    setPinTypeInt(id_end);
    setPinTypeInt(id_op);

    setPinValue(id_start, "0");
    setPinValue(id_step, "1");
    setPinValue(id_end, "1");
    setPinValue(id_op, "0");

    QStringList list;
    list << "Index < End";
    list << "Index <= End";
    list << "Index > End";
    list << "Index >= End";
    list << "Index == End";
    list << "Index != End";
    m_condTip = list;

    m_condOp.push_back(OP_lt);
    m_condOp.push_back(OP_le);
    m_condOp.push_back(OP_gt);
    m_condOp.push_back(OP_ge);
    m_condOp.push_back(OP_eq);
    m_condOp.push_back(OP_ne);
}

bool JZNodeFor::compiler(JZNodeCompiler *c,QString &error)
{
    c->setAutoAddNodeDebug(m_id, false);
    if (!c->addFlowInput(m_id, error))    
        return false;        

    int indexStart = paramIn(0);
    int indexStep = paramIn(1);
    int indexEnd = paramIn(2);
    int indexOut = paramOut(0);    
    int op_id = pinValue(paramIn(3)).toInt();
    int op = m_condOp[op_id];

    bool need_runtime_check = true;
    if (c->isPinLiteral(m_id, indexStart)
        && c->isPinLiteral(m_id, indexEnd)
        && c->isPinLiteral(m_id, indexStep))
    {
        extern bool JZForCheck(int first, int last, int step, int op, QString &error);

        int start = c->pinLiteral(m_id, indexStart).toInt();
        int end = c->pinLiteral(m_id, indexEnd).toInt();
        int step = c->pinLiteral(m_id, indexStep).toInt();

        if (!JZForCheck(start,end,step,op,error))
            return false;
        
        need_runtime_check = false;
    }

    int id_index = c->paramId(m_id,indexStart);
    int id_end = c->paramId(m_id,indexEnd);
    int id_step = c->paramId(m_id, indexStep);    
    int id_out_index = c->paramId(m_id,indexOut);         
    if(need_runtime_check)
    {
        QList<JZNodeIRParam> in, out;
        in << irId(id_index) << irId(id_end) << irId(id_step) << irLiteral(op);
        c->addCall("forRuntimeCheck", in, out);
    }
    c->addSetVariable(irId(id_out_index), irId(id_index));

    //start 
    int start = c->addNodeDebug(m_id);
    c->addCompare(irId(id_index), irId(id_end), op);
    JZNodeIRJmp *jmp_cond_break = new JZNodeIRJmp(OP_jne);
    c->addStatement(JZNodeIRPtr(jmp_cond_break));    
    c->lastStatment()->memo = "body";    
    c->addFlowOutput(m_id);
    c->addJumpSubNode(subFlowOut(0));    

    int continue_pc = c->addExpr(irId(id_index), irId(id_index), irId(id_step), OP_add);
    c->addSetVariable(irId(id_out_index), irId(id_index));
    JZNodeIRJmp *jmp_to_start = new JZNodeIRJmp(OP_jmp);    
    c->addStatement(JZNodeIRPtr(jmp_to_start));
    jmp_to_start->jmpPc = start;
        
    int break_pc = c->addJumpNode(flowOut());        
    jmp_cond_break->jmpPc = break_pc;
    c->setBreakContinue({break_pc},{ continue_pc });

    return true;
}

JZNodePinWidget* JZNodeFor::createWidget(int id)
{
    JZNodePinValueWidget *w = new JZNodePinValueWidget();
    w->initWidget(Type_int, "QComboBox");
    QComboBox *comboBox = qobject_cast<QComboBox*>(w->focusWidget());
    comboBox->addItems(m_condTip);      ;
    comboBox->setCurrentIndex(paramInValue(3).toInt());    

    return w;
}

void JZNodeFor::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);  
}

void JZNodeFor::setRange(int start, int end)
{
    int step = (start < end)? 1: -1;
    setPinValue(paramIn(0), QString::number(start));
    setPinValue(paramIn(1), QString::number(step));
    setPinValue(paramIn(2), QString::number(end));
}

void JZNodeFor::setRange(int start, int step, int end)
{
    setPinValue(paramIn(0), QString::number(start));
    setPinValue(paramIn(1), QString::number(step));
    setPinValue(paramIn(2), QString::number(end));
}

void JZNodeFor::setStart(int start)
{
    setPinValue(paramIn(0), QString::number(start));
}

void JZNodeFor::setStep(int step)
{
    setPinValue(paramIn(1), QString::number(step));
}

void JZNodeFor::setEnd(int end)
{
    setPinValue(paramIn(2), QString::number(end));
}

void JZNodeFor::setOp(int op)
{
    int index = m_condOp.indexOf(op);
    Q_ASSERT(index >= 0);
    setParamInValue(3, QString::number(index));
}

//JZNodeForEach
JZNodeForEach::JZNodeForEach()
{
    m_name = "foreach";
    m_type = Node_foreach;

    addFlowIn();
    int in = addParamIn("");
    addSubFlowOut("loop body", Pin_dispName);
    addFlowOut("complete", Pin_dispName);

    int out1 = addParamOut("key",Pin_dispName);
    int out2 = addParamOut("value",Pin_dispName);
}

JZNodeForEach::~JZNodeForEach()
{

}

bool JZNodeForEach::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addFlowInput(m_id,error))
        return false;
    
    int class_type = c->pinType(m_id,paramIn(0));
    auto obj_inst = environment()->objectManager();
    auto meta = obj_inst->meta(class_type);    

    auto *it_define = meta->function("iterator");
    auto it_meta = obj_inst->meta(it_define->paramOut[0].type);

    int id_list = c->paramId(m_id,paramIn(0));
    JZNodeIRParam list = irId(id_list);    
    JZNodeIRParam it = irId(c->allocStack(it_meta->id));
    JZNodeIRParam itIsEnd = irId(c->allocStack(Type_bool));
            
    auto itNextFunc = it_meta->function("next");
    auto itEndFunc = it_meta->function("atEnd");    
    auto itKeyFunc = it_meta->function("key");
    auto itValueFunc = it_meta->function("value");

    JZNodeIRParam itKey = irId(c->paramId(m_id,paramOut(0)));
    JZNodeIRParam itValue = irId(c->paramId(m_id,paramOut(1)));    

    //it = list.first
    c->addCall(it_define,{list},{it});

    //while(it != it.end()
    int startPc = c->nextPc();
    c->addCall(itEndFunc,{it},{itIsEnd});
    c->addCompare(itIsEnd,irLiteral(true),OP_eq);
    JZNodeIRJmp *jmp_true = new JZNodeIRJmp(OP_je);
    c->addStatement(JZNodeIRPtr(jmp_true));

    c->addCall(itKeyFunc,{it},{itKey});
    c->addCall(itValueFunc,{it},{itValue});

    c->addFlowOutput(m_id);
    c->addJumpSubNode(subFlowOut(0));

    // it++
    int continuePc = c->nextPc();
    c->addCall(itNextFunc,{it},{});
    JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
    jmp->jmpPc = startPc;
    c->addStatement(JZNodeIRPtr(jmp));

    int breakPc = c->addJumpNode(flowOut());
    jmp_true->jmpPc = breakPc;
    c->setBreakContinue({breakPc},{continuePc});

    return true;
}

//JZNodeWhile
JZNodeWhile::JZNodeWhile()
{
    m_name = "while";
    m_type = Node_while;
    addFlowIn();
    addSubFlowOut("loop body", Pin_dispName);
    addFlowOut("complete", Pin_dispName);
    
    int cond = addParamIn("cond", Pin_dispName);
    setPinTypeBool(cond);
}


bool JZNodeWhile::compiler(JZNodeCompiler *c,QString &error)
{
    int continuePc = c->nextPc();
    if(!c->addFlowInput(m_id,error))
        return false;

    int cond = paramIn(0);
    int id = c->paramId(m_id,cond);
    c->addCompare(irId(id),irLiteral(true),OP_eq);

    JZNodeIRJmp *jmp_true = new JZNodeIRJmp(OP_je);
    JZNodeIRJmp *jmp_false = new JZNodeIRJmp(OP_jmp);
    c->addStatement(JZNodeIRPtr(jmp_true));
    c->addStatement(JZNodeIRPtr(jmp_false));    

    int breakPc = c->addJumpNode(flowOut());
    jmp_true->jmpPc = c->addJumpSubNode(subFlowOut(0));
    jmp_false->jmpPc = breakPc;
    
    c->setBreakContinue({breakPc},{continuePc});
    return true;
}

//JZNodeIf
JZNodeIf::JZNodeIf()
{
    m_type = Node_if;
    m_name = "if";
    addFlowIn();
    addFlowOut("complete", Pin_dispName);
    addCondPin();    

    addWidgetIn("Add cond");
    addWidgetIn("Add else");
}

void JZNodeIf::updateCondName()
{
    auto list = paramInList();
    auto flow_list = subFlowList();
    for (int i = 0; i < list.size(); i++)
    {
        setPinName(list[i], "cond" + QString::number(i + 1));        
        setPinName(flow_list[i], "cond" + QString::number(i + 1));
    }
    if (flow_list.size() > list.size())
        setPinName(flow_list.back(), "else");
}

void JZNodeIf::addCondPin()
{
    int in = addParamIn("cond",Pin_dispName);
    setPinTypeBool(in);

    addSubFlowOut("cond", Pin_dispName);    
    updateCondName();    
}

void JZNodeIf::addElsePin()
{
    addSubFlowOut("else", Pin_dispName); 
    updateCondName();
}

int JZNodeIf::pinPri(int id) const
{
    if (pin(id)->name() == "else")
        return Pri_sub_flow + 1;
    else
        return JZNode::pinPri(id);
}

void JZNodeIf::removeCond(int index)
{
    int flow_id = paramInList()[index];
    int in_id = subFlowList()[index];
    removePin(in_id);
    removePin(flow_id);
    updateCondName();
}

void JZNodeIf::removeElse()
{
    int id = subFlowList().back();
    removePin(id);
    updateCondName();
}

int JZNodeIf::btnCondId()
{
    return widgetIn(0);
}

int JZNodeIf::btnElseId()
{
    return widgetIn(1);
}

JZNodePinWidget* JZNodeIf::createWidget(int id)
{
    Q_UNUSED(id);

    JZNodePinButtonWidget *w = new JZNodePinButtonWidget();
    QPushButton *btn = w->button();
    btn->setText(pinName(id));
    if (id == btnCondId())
    {
        btn->connect(btn, &QPushButton::clicked, [this] {
            QByteArray old = toBuffer();
            addCondPin();
            propertyChangedNotify(old);
        });
    }
    else
    {
        btn->connect(btn, &QPushButton::clicked, [this] {
            if (subFlowCount() > paramInCount())
                return;

            QByteArray old = toBuffer();
            addElsePin();
            propertyChangedNotify(old);
        });
    }
    return w;
}

QStringList JZNodeIf::pinActionList(int id)
{
    int param_index = paramInList().indexOf(id);
    int sub_index = subFlowList().indexOf(id);
    if (param_index == -1 && sub_index == -1)
        return QStringList();

    bool isElse = (subFlowCount() > paramInCount()) && (sub_index == subFlowCount() - 1);

    QStringList ret;
    if (paramInCount() > 1 || isElse)
        ret.push_back("删除");

    return ret;
}

bool JZNodeIf::pinActionTriggered(int id, int)
{
    int pin_index = paramInList().indexOf(id);
    if(pin_index == -1)
        pin_index = subFlowList().indexOf(id);

    bool isElse = (subFlowCount() > paramInCount()) && (pin_index == subFlowCount() - 1);
    if (isElse)
        removeElse();
    else
        removeCond(pin_index);

    return true;
}

bool JZNodeIf::compiler(JZNodeCompiler *c, QString &error) 
{
    QList<int> breakPc,continuePc;
    bool isElse = subFlowCount() > paramInCount();
    
    JZNodeIRJmp *last_jmp = nullptr;
    QList<int> inList = paramInList();
    for (int i = 0; i < inList.size(); i++)
    {
        int nextPc = c->nextPc();
        if (!c->addFlowInput(m_id, inList[i], error))
            return false;
        if (last_jmp)
            last_jmp->jmpPc = nextPc;

        int cond = c->paramId(m_id, inList[i]);
        c->addCompare(irId(cond), irLiteral(false), OP_eq);

        JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_je);
        c->addStatement(JZNodeIRPtr(jmp));
        last_jmp = jmp;

        c->addJumpSubNode(subFlowOut(i));        
    }    
    if (isElse)
    {
        int else_pc = c->addJumpSubNode(subFlowOut(inList.size()));
        last_jmp->jmpPc = else_pc;
    }    
    int ret = c->addJumpNode(flowOut(0));    
    if(!isElse)
        last_jmp->jmpPc = ret;

    int sub_flow_count = subFlowCount();
    for (int i = 0; i < sub_flow_count; i++)
    {
        continuePc << ret;
        breakPc << -1;
    }
    c->setBreakContinue(breakPc, continuePc);
    return true;
}

//JZNodeSwitch
JZNodeSwitch::JZNodeSwitch()
{
    m_type = Node_switch;
    m_name = "switch";
    m_caseType << JZNodeType::typeName(Type_int) << JZNodeType::typeName(Type_string);

    addFlowIn();
    addFlowOut("complete",Pin_dispName);
    int in = addParamIn("cond", Pin_dispName);    
    setPinType(in, m_caseType);

    addParamOut("cond", Pin_dispName);
    addCase();

    m_btnCase = addWidgetOut("Add case");
    m_btnDefault = addWidgetOut("Add default");
}

void JZNodeSwitch::addCase()
{
    int sub_id = addSubFlowOut("case", Pin_dispName | Pin_dispValue | Pin_editValue);
    setPinType(sub_id, m_caseType);
}

void JZNodeSwitch::addDefault()
{
    addSubFlowOut("default", Pin_dispName);    
}

int JZNodeSwitch::pinPri(int id) const
{
    if(pin(id)->name() == "default")
        return Pri_sub_flow + 1;
    else
        return JZNode::pinPri(id);
}

void JZNodeSwitch::removeCase(int index)
{
    int id = subFlowList()[index];
    removePin(id);
}

void JZNodeSwitch::removeDefault()
{
    int id = subFlowList().back();
    removePin(id);    
}

int JZNodeSwitch::caseCount()
{
    auto list = subFlowList();
    int id = list.back();
    bool isDefault = !(pin(id)->flag() & Pin_editValue);
    if (isDefault)
        return list.size() - 1;
    else
        return list.size();
}

void JZNodeSwitch::setCaseValue(int index, const QString &v)
{
    Q_ASSERT(index < caseCount());
    pin(subFlowOut(index))->setValue(v);
}

JZNodePinWidget* JZNodeSwitch::createWidget(int pin_id)
{    
    JZNodePinButtonWidget *w = new JZNodePinButtonWidget();
    QPushButton *btn = w->button();
    btn->setText(pinName(pin_id));
    if (pin_id == widgetOut(0))
    {
        btn->connect(btn, &QPushButton::clicked, [this] {
            QByteArray old = toBuffer();
            addCase();
            propertyChangedNotify(old);
        });
    }
    else
    {
        btn->connect(btn, &QPushButton::clicked, [this] {
            if (subFlowCount() > paramInCount())
                return;

            int id = subFlowList().back();
            bool isDefault = !(pin(id)->flag() & Pin_editValue);
            if (isDefault)
                return;

            addDefault();
        });
    }

    return w;
}

QStringList JZNodeSwitch::pinActionList(int id)
{    
    int sub_index = subFlowList().indexOf(id);
    if (sub_index == -1)
        return QStringList();

    bool isDefault = !(pin(id)->flag() & Pin_editValue);

    QStringList ret;
    if (caseCount() > 1 || isDefault)
        ret.push_back("删除");

    return ret;
}

bool JZNodeSwitch::pinActionTriggered(int id, int index)
{
    int pin_index = subFlowList().indexOf(id);

    bool isDefault = !(pin(id)->flag() & Pin_editValue);
    if (isDefault)
        removeDefault();
    else
        removeCase(pin_index);

    return true;
}

bool JZNodeSwitch::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    int case_idx = 0;    
    auto sub_flow_list = subFlowList();
    if (sub_flow_list.size() == 0) 
    {
        error = "请添加case";
        return false;
    }
    
    auto env = environment();
    int in_id = c->paramId(m_id, paramIn(0));
    int out_id = c->paramId(m_id, paramOut(0));    
    c->setPinType(m_id,paramOut(0),c->pinType(m_id,paramIn(0)));
    c->addSetVariable(irId(out_id), irId(in_id));
    c->addFlowOutput(m_id);

    int case_count = sub_flow_list.size();
    if (pin(sub_flow_list.back())->name() == "default")
        case_count--;

    int in_type = c->pinType(m_id, paramIn(0));

    JZNodeIRJmp *pre_jmp = nullptr;
    for (case_idx = 0; case_idx < case_count; case_idx++)
    {
        QString out_value_str = pin(sub_flow_list[case_idx])->value();
        QVariant value = env->initValue(in_type, out_value_str);

        int jmp_cmp = c->addCompare(irId(in_id), irLiteral(value), OP_eq);
        JZNodeIRJmp *jmp_true = new JZNodeIRJmp(OP_je);
        JZNodeIRJmp *jmp_false = new JZNodeIRJmp(OP_jmp);
        c->addStatement(JZNodeIRPtr(jmp_true));
        c->addStatement(JZNodeIRPtr(jmp_false));
        int jmp = c->addJumpSubNode(sub_flow_list[case_idx]);
        jmp_true->jmpPc = jmp;
        if (pre_jmp)
            pre_jmp->jmpPc = jmp_cmp;
        pre_jmp = jmp_false;
    }
    for (; case_idx < sub_flow_list.size(); case_idx++)
    {
        int jmp = c->addJumpSubNode(sub_flow_list[case_idx]);
        if (pre_jmp)
            pre_jmp->jmpPc = jmp;
    }        
    int out_pc = c->addJumpNode(flowOut());
    QList<int> breakPc, continuePc;
    for (int i = 0; i < sub_flow_list.size(); i++)
    {
        breakPc.push_back(out_pc);
        continuePc.push_back(out_pc);
    }
    c->setBreakContinue(breakPc, continuePc);

    return true;
}

//JZNodeBranch
JZNodeBranch::JZNodeBranch()
{
    m_name = "branch";
    m_type = Node_branch;
    addFlowIn();
    addFlowOut("true",Pin_dispName);
    addFlowOut("false",Pin_dispName);
    
    int cond = addParamIn("cond",Pin_dispName);
    setPinTypeBool(cond);
}

bool JZNodeBranch::compiler(JZNodeCompiler *c,QString &error)
{
    int cond = paramIn(0);
    if(!c->addFlowInput(m_id,error))
        return false;

    int id = c->paramId(m_id,cond);
    c->addCompare(irId(id),irLiteral(true),OP_eq);

    JZNodeIRJmp *jmp_true = new JZNodeIRJmp(OP_je);
    JZNodeIRJmp *jmp_false = new JZNodeIRJmp(OP_jmp);
    c->addStatement(JZNodeIRPtr(jmp_true));
    c->addStatement(JZNodeIRPtr(jmp_false));
    
    jmp_true->jmpPc = c->addJumpNode(flowOut(0));
    jmp_false->jmpPc = c->addJumpNode(flowOut(1));
    return true;
}

//JZNodeAssert
JZNodeAssert::JZNodeAssert()
{
    m_name = "assert";
    m_type = Node_assert;
    addFlowIn();
    addFlowOut();    

    int cond = addParamIn("cond", Pin_dispName);
    setPinTypeBool(cond);

    int tips = addParamIn("tips", Pin_dispName | Pin_dispValue | Pin_editValue);
    setPinTypeString(tips);
}

bool JZNodeAssert::compiler(JZNodeCompiler *c, QString &error)
{
    int cond = paramIn(0);
    if (!c->addFlowInput(m_id, error))
        return false;

    int id = c->paramId(m_id, cond);
    int id_tips = c->paramId(m_id, paramIn(1));
    c->addCompare(irId(id), irLiteral(true), OP_eq);
    c->addAssert(irId(id_tips));
    return true;
}

//JZNodeTryCatch
JZNodeTryCatch::JZNodeTryCatch()
{
    addFlowIn();
    addFlowOut("complete",Pin_dispName);
    addSubFlowOut("try");
    addSubFlowOut("catch");
    addSubFlowOut("finally");
}

bool JZNodeTryCatch::compiler(JZNodeCompiler *compiler, QString &error)
{
    return true;
}

//JZNodeMainLoop
JZNodeMainLoop::JZNodeMainLoop()
{        
    m_type = Node_mainLoop;
    m_name = "MainLoop";

    addFlowIn();
    int in = addParamIn("window",Pin_dispName);
    setPinType(in, { JZNodeType::typeName(Type_widget) });
}

JZNodeMainLoop::~JZNodeMainLoop()
{    
}

bool JZNodeMainLoop::compiler(JZNodeCompiler *c, QString &error)
{
    if(!c->addFlowInput(m_id,error))
        return false;

    int id = c->paramId(m_id,paramIn(0));
    c->addAlloc(JZNodeIRAlloc::Heap,"__mainwindow__", Type_widget);

    JZNodeIRSet *ir_set = new JZNodeIRSet();    
    ir_set->dst = irRef("__mainwindow__");
    ir_set->src = irId(id);
    c->addStatement(JZNodeIRPtr(ir_set));

    JZNodeIR *ir_return = new JZNodeIR(OP_return);
    c->addStatement(JZNodeIRPtr(ir_return));

    return true;    
}