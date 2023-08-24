#include "JZNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"

// JZNodeGemo
JZNodeGemo::JZNodeGemo()
{
    nodeId = -1;
    propId = -1;
}

JZNodeGemo::JZNodeGemo(int id, int prop)
{
    nodeId = id;
    propId = prop;
}

bool JZNodeGemo::isNull() const
{
    return (nodeId == -1);
}

bool JZNodeGemo::operator==(const JZNodeGemo &other) const
{
    return nodeId == other.nodeId && propId == other.propId;
}

// JZNodeConnect
JZNodeConnect::JZNodeConnect()
{
    id = -1;
}

QDataStream &operator<<(QDataStream &s, const JZNodeConnect &param)
{
    s << param.id;
    s << param.from.nodeId;
    s << param.from.propId;
    s << param.to.nodeId;
    s << param.to.propId;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeConnect &param)
{
    s >> param.id;
    s >> param.from.nodeId;
    s >> param.from.propId;
    s >> param.to.nodeId;
    s >> param.to.propId;
    return s;
}

JZNodeConnect parseLine(const QByteArray &buffer)
{
    JZNodeConnect line;
    QDataStream s(buffer);
    s >> line;
    return line;
}

QByteArray formatLine(const JZNodeConnect &line)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << line;
    return data;
}

// JZNode
JZNode::JZNode()
{
    m_id = INVALID_ID;
    m_file = nullptr;
    m_flag = Node_propNone;
    m_type = Node_none;
}

JZNode::~JZNode()
{
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

bool JZNode::isFlowNode() const
{
    return propCount(Prop_flow) > 0;
}

int JZNode::addProp(const JZNodePin &prop)
{
    Q_ASSERT(prop.isInput() || prop.isOutput());
    Q_ASSERT(prop.isFlow() || prop.isParam() || prop.isSubFlow() || prop.isButton());

    int max_id = 0;
    for (int i = 0; i < m_propList.size(); i++)
        max_id = qMax(max_id, m_propList[i].id() + 1);

    JZNodePin new_prop = prop;
    new_prop.setId(max_id);
    m_propList.push_back(new_prop);
    return max_id;
}

int JZNode::addParamIn(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Prop_in | Prop_param | extFlag);
    return addProp(pin);        
}

int JZNode::addParamOut(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Prop_out | Prop_param | extFlag);
    return addProp(pin);
}

int JZNode::addFlowIn(int extFlag)
{
    JZNodePin pin;
    pin.setFlag(Prop_in | Prop_flow | extFlag);
    return addProp(pin);
}

int JZNode::addFlowOut(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Prop_out | Prop_flow | extFlag);
    return addProp(pin);
}

int JZNode::addSubFlowOut(QString name,int extFlag)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Prop_out | Prop_subFlow | extFlag);
    return addProp(pin);
}

void JZNode::removeProp(int id)
{
    int index = indexOfProp(id);
    if (index == -1)
        return;

    m_propList.removeAt(id);
}

JZNodePin *JZNode::prop(int id)
{
    int index = indexOfProp(id);
    if (index >= 0)
        return &m_propList[index];
    else
        return nullptr;
}

const JZNodePin *JZNode::prop(int id) const
{
    int index = indexOfProp(id);
    if (index >= 0)
        return &m_propList[index];
    else
        return nullptr;
}

JZNodePin *JZNode::prop(QString name)
{
    int index = indexOfPropByName(name);
    if (index >= 0)
        return &m_propList[index];
    else
        return nullptr;
}

bool JZNode::hasProp(int id) const
{
    return indexOfProp(id) >= 0;
}

int JZNode::indexOfProp(int id) const
{
    for (int i = 0; i < m_propList.size(); i++)
    {
        if (m_propList[i].id() == id)
            return i;
    }
    return -1;
}

int JZNode::indexOfPropByName(QString name) const
{
    for (int i = 0; i < m_propList.size(); i++)
    {
        if (m_propList[i].name() == name)
            return i;
    }
    return -1;
}

int JZNode::indexOfPropByType(int id, int flag) const
{
    int type_index = 0;
    for (int i = 0; i < m_propList.size(); i++)
    {
        if (m_propList[i].id() == id)
            return type_index;
        if (m_propList[i].flag() & flag)
            type_index++;
    }
    return -1;
}

QVector<int> JZNode::propInList(int flag) const
{
    return propListByType(Prop_in | flag);
}

QVector<int> JZNode::propOutList(int flag) const
{
    return propListByType(Prop_out | flag);
}

QVector<int> JZNode::propListByType(int flag) const
{
    QVector<int> ret;
    for (int i = 0; i < m_propList.size(); i++)
    {
        if((m_propList[i].flag() & flag) == flag)
            ret.push_back(m_propList[i].id());
    }
    return ret;
}

int JZNode::propCount(int flag) const
{
    int count = 0;
    for (int i = 0; i < m_propList.size(); i++)
    {
        if ((m_propList[i].flag() & flag) == flag)
            count++;
    }
    return count;
}

int JZNode::paramIn(int index) const
{
    auto list = propInList(Prop_param);
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
    return propInList(Prop_param).size();
}

QVector<int> JZNode::paramInList() const
{
    return propInList(Prop_param);
}

QVariant JZNode::paramInValue(int index) const
{
    auto list = paramInList();
    return propValue(list[index]);
}

void JZNode::setParamInValue(int index, const QVariant &value)
{
    auto list = paramInList();    
    setPropValue(list[index], value);
}

int JZNode::paramOut(int index) const
{
    auto list = propOutList(Prop_param);
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
    return propOutList(Prop_param).size();
}

QVector<int> JZNode::paramOutList() const
{
    return propOutList(Prop_param);
}

QVariant JZNode::paramOutValue(int index) const
{
    auto list = paramOutList();
    return propValue(list[index]);
}

void JZNode::setParamOutValue(int index, const QVariant &value)
{
    auto list = paramOutList();
    setPropValue(list[index], value);
}

int JZNode::flowIn() const
{
    auto list = propInList(Prop_flow);
    if(list.size() != 0)
        return list[0];
    else
        return -1;
}

JZNodeGemo JZNode::flowInGemo() const
{
    return JZNodeGemo(m_id,flowIn());
}

int JZNode::flowOut(int index) const
{
    auto list = propOutList(Prop_flow);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::flowOutGemo(int index) const
{
    return JZNodeGemo(m_id,flowOut(index));
}

QVector<int> JZNode::flowOutList() const
{
    return propOutList(Prop_flow);
}

int JZNode::flowOutCount() const
{
    return propOutList(Prop_flow).size();
}

int JZNode::subFlowOut(int index) const
{
    auto list = propOutList(Prop_subFlow);
    if(index < list.size())
        return list[index];
    else
        return -1;
}

JZNodeGemo JZNode::subFlowOutGemo(int index) const
{
    return JZNodeGemo(m_id,subFlowOut(index));
}

QVector<int> JZNode::subFlowList() const
{
    return propOutList(Prop_subFlow);
}

int JZNode::subFlowCount() const
{
    return propOutList(Prop_subFlow).size();
}

int JZNode::addButtonIn(QString name)
{
    JZNodePin btn;
    btn.setName(name);
    btn.setFlag(Prop_button | Prop_in | Prop_dispName);
    return addProp(btn);    
}

int JZNode::addButtonOut(QString name)
{
    JZNodePin btn;
    btn.setName(name);
    btn.setFlag(Prop_button | Prop_out | Prop_dispName);
    return addProp(btn);
}

QVariant JZNode::propValue(int id) const
{
    auto pin = prop(id);
    Q_ASSERT(pin);

    return pin->value();
}

void JZNode::setPropValue(int id,QVariant value)
{        
    auto pin = prop(id);
    Q_ASSERT(pin);

    pin->setValue(value);
    if(m_file)
        pinChanged(id);
    else
        m_notifyList << id;
}

QString JZNode::propName(int id) const
{
    auto pin = prop(id);
    Q_ASSERT(pin);

    return pin->name();
}

void JZNode::setPropName(int id,QString name)
{
    auto pin = prop(id);
    Q_ASSERT(pin);

    pin->setName(name);
    if(m_file)
        pinChanged(id);
    else
        m_notifyList << id;
}

QVector<int> JZNode::propList() const
{
    QVector<int> result;
    for(int i = 0; i < m_propList.size(); i++)
        result.push_back(m_propList[i].id());

    return result;
}

void JZNode::setVariable(const QString &name)
{
    Q_UNUSED(name);
}

QString JZNode::variable() const
{
    return QString();
}

int JZNode::variableType() const
{
    return Type_any;
}

JZScriptFile *JZNode::file() const
{
    return m_file;
}

void JZNode::setFile(JZScriptFile *file)
{
    m_file = file;
    fileInitialized();

    //delay notify
    for(int i = 0; i < m_notifyList.size(); i++)
        pinChanged(m_notifyList[i]);
    m_notifyList.clear();
}

QString JZNode::name() const
{
    return m_name;
}

void JZNode::setName(QString name)
{
    m_name = name;
}

bool JZNode::canRemove()
{
    return !(m_flag & Node_propNoRemove);
}

bool JZNode::canDragVariable()
{
    return (m_flag & Node_propDragVariable);
}

int JZNode::id() const
{
    return m_id;
}

void JZNode::setId(int id)
{
    m_id = id;
}

QList<int> JZNode::propType(int id)
{
    return prop(id)->dataType();
}

QMap<int,int> JZNode::calcPropOutType(const QMap<int,int> &inType)
{
    QMap<int,int> types;
    auto list = paramOutList();
    for(int i = 0; i < list.size(); i++)
    {
        auto pin = prop(list[i]);
        int dataType = Type_none;
        if(pin->dataType().size() > 0)
            dataType = pin->dataType().front();
        types[list[i]] = dataType;
    }
    return types;
}

void JZNode::setPinTypeAny(int id)
{
    prop(id)->setDataType({Type_any});
}

void JZNode::setPinTypeInt(int id)
{
    prop(id)->setDataType({Type_int,Type_int64});
}

void JZNode::setPinTypeNumber(int id)
{
    prop(id)->setDataType({Type_bool,Type_int,Type_int64,Type_double});
}

void JZNode::setPinTypeBool(int id)
{
    prop(id)->setDataType({Type_bool});
}

void JZNode::setPinTypeString(int id)
{
    prop(id)->setDataType({Type_string});
}

void JZNode::setPinType(int id,const QList<int> &type)
{
    prop(id)->setDataType(type);
}

void JZNode::drag(const QVariant &v)
{
    Q_UNUSED(v);
}

bool JZNode::pinClicked(int id)
{
    Q_UNUSED(id);
    return false;
}

bool JZNode::pinAction(int id)
{
    Q_UNUSED(id);
    return false;
}

void JZNode::fileInitialized()
{

}

void JZNode::pinLinked(int id)
{
    Q_UNUSED(id);
}

void JZNode::pinUnlinked(int id)
{
    Q_UNUSED(id);
}

void JZNode::pinChanged(int id)
{
    Q_UNUSED(id);
}

void JZNode::saveToStream(QDataStream &s) const
{
    s << m_id;
    s << m_name;
    s << m_flag;    
    s << m_propList;
    s << m_notifyList;
}

void JZNode::loadFromStream(QDataStream &s)
{
    s >> m_id;
    s >> m_name;
    s >> m_flag;
    s >> m_propList;
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

void JZNodeReturn::setFunction(const FunctionDefine *def)
{
    auto inList = paramInList();
    for (int i = 0; i < inList.size(); i++)
        removeProp(inList[i]);

    for (int i = 0; i < def->paramOut.size(); i++)
    {
        int in = addParamIn(def->paramOut[i].name, Prop_dispName | Prop_dispValue | Prop_editValue);
        setPinType(in, { def->paramOut[i].dataType });
    }
}

bool JZNodeReturn::compiler(JZNodeCompiler *c,QString &error)
{   
    if(!c->addFlowInput(m_id,error))
        return false;     

    auto inList = paramInList();
    for(int i = 0; i < inList.size(); i++)
    {
        int id = c->paramId(m_id,inList[i]);
        c->addSetVariable(irId(Reg_Call+i),irId(id));
    }
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

bool JZNodeExit::compiler(JZNodeCompiler *compiler,QString &error)
{
    JZNodeIR ir(Node_exit);    
    return true;
}

//JZNodeSequence
JZNodeSequence::JZNodeSequence()
{
    m_name = "sequence";
    m_type = Node_sequence;
    addFlowIn();
    addFlowOut("complete",Prop_dispName);

    addSequeue();

    JZNodePin btn;
    btn.setName("Add pin");
    btn.setFlag(Prop_button | Prop_out | Prop_dispName);
    addProp(btn);           
}

int JZNodeSequence::addSequeue()
{
    return addSubFlowOut("Seqeue " + QString::number(subFlowCount() + 1),Prop_dispName);
}

void JZNodeSequence::removeSequeue(int id)
{
    removeProp(id);
    auto list = subFlowList();
    for(int i = 0; i < list.size(); i++)
    {
        QString name = "Seqeue " + QString::number(subFlowCount() + 1);
        prop(list[i])->setName(name);
    }
}

bool JZNodeSequence::compiler(JZNodeCompiler *c,QString &error)
{
    QList<int> breakList;
    QList<int> continueList;

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

bool JZNodeSequence::pinClicked(int id)
{
    addSequeue();
    return true;
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
    addSubFlowOut("loop body",Prop_dispName);
    addFlowOut("complete",Prop_dispName);

    int id_start = addParamIn("First index",Prop_editValue | Prop_dispName | Prop_dispValue);
    int id_step = addParamIn("Step", Prop_editValue | Prop_dispName | Prop_dispValue);
    int id_end = addParamIn("End index",Prop_editValue | Prop_dispName | Prop_dispValue);
    int id_index = addParamOut("index", Prop_dispName);
    setPinTypeInt(id_start);
    setPinTypeInt(id_step);
    setPinTypeInt(id_index);
    setPinTypeInt(id_end);
}

bool JZNodeFor::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addFlowInput(m_id,error))
        return false;

    int indexStart = paramIn(0);
    int indexStep = paramIn(1);
    int indexEnd = paramIn(2);
    int indexOut = paramOut(0);    

    int id_start = c->paramId(m_id,indexStart);
    int id_end = c->paramId(m_id,indexEnd);
    int id_step = c->paramId(m_id, indexStep);
    int id_index = c->paramId(m_id,indexOut);

    int id_jmp = c->allocStack();
    c->addCompare(irId(id_start), irId(id_end), OP_lt);
    JZNodeIRJmp *jmp_set1 = new JZNodeIRJmp(OP_jne);    
    c->addStatement(JZNodeIRPtr(jmp_set1));

    JZNodeIRSet *op_set1 = new JZNodeIRSet();
    op_set1->dst = irId(id_jmp);
    c->addStatement(JZNodeIRPtr(op_set1));
    JZNodeIRJmp *jmp_set2 = new JZNodeIRJmp(OP_jmp);
    c->addStatement(JZNodeIRPtr(jmp_set2));    

    JZNodeIRSet *op_set2 = new JZNodeIRSet();
    op_set2->dst = irId(id_jmp);    
    int pc_set_2 = c->addStatement(JZNodeIRPtr(op_set2));
    jmp_set1->jmpPc = irLiteral(pc_set_2);
    
    int set_index_pc = c->addSetVariable(irId(id_index),irId(id_start));
    jmp_set2->jmpPc = irLiteral(set_index_pc);

    //有jmp_to_cmp保底，可以直接加1
    int continue_next_pc = c->currentPc() + 1;
    c->addFlowOutput(m_id);    
    JZNodeIRJmp *jmp_to_cmp = new JZNodeIRJmp(OP_jmp);
    jmp_to_cmp->jmpPc = irId(id_jmp);
    c->addStatement(JZNodeIRPtr(jmp_to_cmp));

    // start < end
    int cmp_lt = c->addCompare(irId(id_index),irId(id_end), OP_lt);
    op_set1->src = irLiteral(cmp_lt);
    JZNodeIRJmp *jmp_false_lt = new JZNodeIRJmp(OP_jne);
    c->addStatement(JZNodeIRPtr(jmp_false_lt));
    
    JZNodeIRJmp *jmp_to_body1 = new JZNodeIRJmp(OP_jmp);
    c->addStatement(JZNodeIRPtr(jmp_to_body1));

    // start > end
    int cmp_gt = c->addCompare(irId(id_index), irId(id_end), OP_gt);
    op_set2->src = irLiteral(cmp_gt);
    JZNodeIRJmp *jmp_false_ge = new JZNodeIRJmp(OP_jne);
    c->addStatement(JZNodeIRPtr(jmp_false_ge));

    JZNodeIRJmp *jmp_to_body2 = new JZNodeIRJmp(OP_jmp);
    c->addStatement(JZNodeIRPtr(jmp_to_body2));

    int body_pc = c->addJumpSubNode(subFlowOut(0));
    jmp_to_body1->jmpPc = irLiteral(body_pc);
    jmp_to_body2->jmpPc = irLiteral(body_pc);

    int continuePc = c->addExpr(irId(id_index),irId(id_index), irId(id_step),OP_add);
    JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);       
    jmp->jmpPc = irLiteral(continue_next_pc);
    c->addStatement(JZNodeIRPtr(jmp));
    
    int breakPc = c->addJumpNode(flowOut());
    jmp_false_lt->jmpPc = irLiteral(breakPc);
    jmp_false_ge->jmpPc = irLiteral(breakPc);
    c->setBreakContinue({breakPc},{continuePc});    

    return true;
}

void JZNodeFor::setRange(int start, int end)
{
    int step = (start < end)? 1: -1;
    setPropValue(paramIn(0), start);
    setPropValue(paramIn(1), step);
    setPropValue(paramIn(2), end);
}

void JZNodeFor::setRange(int start, int end, int step)
{
    setPropValue(paramIn(0), start);
    setPropValue(paramIn(1), step);
    setPropValue(paramIn(2), end);
}

void JZNodeFor::setStart(int start)
{
    setPropValue(paramIn(0), start);
}

void JZNodeFor::setStep(int step)
{
    setPropValue(paramIn(1), step);
}

void JZNodeFor::setEnd(int end)
{
    setPropValue(paramIn(2), end);
}

//JZNodeForEach
JZNodeForEach::JZNodeForEach()
{
    m_name = "foreach";
    m_type = Node_foreach;

    addFlowIn();
    int in = addParamIn("");
    addSubFlowOut("loop body", Prop_dispName);
    addFlowOut("complete", Prop_dispName);

    int out1 = addParamOut("key",Prop_dispName);
    int out2 = addParamOut("value",Prop_dispName);
    prop(in)->setDataType({Type_list,Type_map});
    setPinTypeAny(out1);
    setPinTypeAny(out2);
}

JZNodeForEach::~JZNodeForEach()
{

}

bool JZNodeForEach::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addFlowInput(m_id,error))
        return false;

    int id_list = c->paramId(m_id,paramIn(0));
    JZNodeIRParam list = irId(id_list);
    JZNodeIRParam className = irId(c->allocStack());
    JZNodeIRParam it = irId(c->allocStack());
    JZNodeIRParam itName = irId(c->allocStack());
    JZNodeIRParam itBeginFunc = irId(c->allocStack());
    JZNodeIRParam itNextFunc = irId(c->allocStack());
    JZNodeIRParam itEndFunc = irId(c->allocStack());
    JZNodeIRParam itIsEnd = irId(c->allocStack());
    JZNodeIRParam itKeyFunc = irId(c->allocStack());
    JZNodeIRParam itValueFunc = irId(c->allocStack());
    JZNodeIRParam itKey = irId(c->paramId(m_id,paramOut(0)));
    JZNodeIRParam itValue = irId(c->paramId(m_id,paramOut(1)));    

    c->addCall(irLiteral("typename"),{list},{className});
    c->addCall(irLiteral("String.append"),{className,irLiteral(".iterator")},{itBeginFunc});

    //it = list.first
    c->addCall(itBeginFunc,{list},{it});

    c->addCall(irLiteral("typename"),{it},{itName});
    c->addCall(irLiteral("String.append"),{itName,irLiteral(".next")},{itNextFunc});
    c->addCall(irLiteral("String.append"),{itName,irLiteral(".atEnd")},{itEndFunc});
    c->addCall(irLiteral("String.append"),{itName,irLiteral(".key")},{itKeyFunc});
    c->addCall(irLiteral("String.append"),{itName,irLiteral(".value")},{itValueFunc});

    //while(it != it.end()
    int startPc = c->currentPc() + 1;
    c->addCall(itEndFunc,{it},{itIsEnd});
    c->addCompare(itIsEnd,irLiteral(true),OP_eq);
    JZNodeIRJmp *jmp_true = new JZNodeIRJmp(OP_je);
    c->addStatement(JZNodeIRPtr(jmp_true));

    c->addCall(itKeyFunc,{it},{itKey});
    c->addCall(itValueFunc,{it},{itValue});

    c->addFlowOutput(m_id);
    c->addJumpSubNode(subFlowOut(0));

    // it++
    int continuePc = c->currentPc() + 1;
    c->addCall(itNextFunc,{it},{});
    JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
    jmp->jmpPc = irLiteral(startPc);
    c->addStatement(JZNodeIRPtr(jmp));

    int breakPc = c->addJumpNode(flowOut());
    jmp_true->jmpPc = irLiteral(breakPc);
    c->setBreakContinue({breakPc},{continuePc});

    return true;
}

//JZNodeWhile
JZNodeWhile::JZNodeWhile()
{
    m_name = "while";
    m_type = Node_while;
    addFlowIn();
    addSubFlowOut("loop body", Prop_dispName);
    addFlowOut("complete", Prop_dispName);
    
    int cond = addParamIn("cond", Prop_dispName);
    setPinTypeBool(cond);
}


bool JZNodeWhile::compiler(JZNodeCompiler *c,QString &error)
{
    int continuePc = c->currentPc();    
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
    jmp_true->jmpPc = irLiteral(c->addJumpSubNode(subFlowOut(0)));
    jmp_false->jmpPc = irLiteral(breakPc);
    
    c->setBreakContinue({breakPc},{continuePc});
    return true;
}

//JZNodeIf
JZNodeIf::JZNodeIf()
{
    m_type = Node_if;
    m_name = "if";
    addFlowIn();
    addFlowOut();
    addCondPin();    

    m_btnCond = addButtonIn("Add cond");
    m_btnElse = addButtonIn("Add else");
}

void JZNodeIf::addCondPin()
{
    int in = addParamIn("cond",Prop_dispName);
    setPinTypeBool(in);

    addSubFlowOut("cond");
}

void JZNodeIf::addElsePin()
{
    addSubFlowOut("else");
}

bool JZNodeIf::pinClicked(int id)
{
    if (id == m_btnCond)
        addCondPin();
    else if (id == m_btnElse)
        addElsePin();

    return true;
}

bool JZNodeIf::compiler(JZNodeCompiler *c, QString &error) 
{
    QList<int> breakPc,continuePc;
    bool isElse = subFlowCount() > paramInCount();
    
    JZNodeIRJmp *last_jmp = nullptr;
    QVector<int> inList = paramInList();
    for (int i = 0; i < inList.size(); i++)
    {
        int nextPc = c->currentPc() + 1;
        if (!c->addFlowInput(m_id, inList[i], error))
            return false;
        if (last_jmp)
            last_jmp->jmpPc = irLiteral(nextPc);

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
        last_jmp->jmpPc = irLiteral(else_pc);
    }    
    int ret = c->addJumpNode(flowOut(0));    
    if(!isElse)
        last_jmp->jmpPc = irLiteral(ret);

    int sub_flow_count = subFlowCount();
    for (int i = 0; i < sub_flow_count; i++)
    {
        continuePc << ret;
        breakPc << 1;
    }
    c->setBreakContinue(breakPc, continuePc);
    return true;
}

//JZNodeSwitch
JZNodeSwitch::JZNodeSwitch()
{
    m_type = Node_switch;
    m_name = "switch";
    addFlowIn();
    addFlowOut("complete",Prop_dispName);
    int in = addParamIn("cond", Prop_dispName);    
    setPinType(in, { Type_int,Type_string });

    addParamOut("cond", Prop_dispName);

    m_btnCase = addButtonOut("Add case");
    m_btnDefault = addButtonOut("Add default");
}

void JZNodeSwitch::addCase()
{
    addSubFlowOut("case", Prop_dispName | Prop_dispValue | Prop_editValue);
}

void JZNodeSwitch::addDefault()
{
    addSubFlowOut("default", Prop_dispName);
}

void JZNodeSwitch::setCaseValue(int index, const QVariant &v)
{
    prop(subFlowOut(index))->setValue(v);
}

bool JZNodeSwitch::pinClicked(int id)
{
    if (id == m_btnCase)
        addCase();
    else if (id == m_btnDefault)
        addDefault();

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

    int in_id = c->paramId(m_id, paramIn(0));
    int out_id = c->paramId(m_id, paramOut(0));
    
    c->addSetVariable(irId(out_id), irId(in_id));
    c->addFlowOutput(m_id);

    int case_count = sub_flow_list.size();
    if (prop(sub_flow_list.back())->name() == "default")
        case_count--;

    JZNodeIRJmp *pre_jmp = nullptr;
    for (case_idx = 0; case_idx < case_count; case_idx++)
    {
        auto out_value = prop(sub_flow_list[case_idx])->value();

        int jmp_cmp = c->addCompare(irId(in_id), irLiteral(out_value), OP_eq);
        JZNodeIRJmp *jmp_true = new JZNodeIRJmp(OP_je);
        JZNodeIRJmp *jmp_false = new JZNodeIRJmp(OP_jmp);
        c->addStatement(JZNodeIRPtr(jmp_true));
        c->addStatement(JZNodeIRPtr(jmp_false));
        int jmp = c->addJumpSubNode(sub_flow_list[case_idx]);
        jmp_true->jmpPc = irLiteral(jmp);
        if (pre_jmp)
            pre_jmp->jmpPc = irLiteral(jmp_cmp);
        pre_jmp = jmp_false;
    }
    for (; case_idx < sub_flow_list.size(); case_idx++)
    {
        int jmp = c->addJumpSubNode(sub_flow_list[case_idx]);
        if (pre_jmp)
            pre_jmp->jmpPc = irLiteral(jmp);
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
    addFlowOut("true",Prop_dispName);
    addFlowOut("false",Prop_dispName);
    
    int cond = addParamIn("cond",Prop_dispName);
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
    
    jmp_true->jmpPc = irLiteral(c->addJumpNode(flowOut(0)));
    jmp_false->jmpPc = irLiteral(c->addJumpNode(flowOut(1)));
    return true;
}

//JZNodeAssert
JZNodeAssert::JZNodeAssert()
{
    m_name = "assert";
    m_type = Node_assert;
    addFlowIn();
    addFlowOut();    

    int cond = addParamIn("cond", Prop_dispName);
    setPinTypeBool(cond);

    int tips = addParamIn("tips", Prop_dispName | Prop_dispValue | Prop_editValue);
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
    addFlowOut("complete",Prop_dispName);
    addSubFlowOut("try");
    addSubFlowOut("catch");
    addSubFlowOut("finally");
}

bool JZNodeTryCatch::compiler(JZNodeCompiler *compiler, QString &error)
{
    return true;
}