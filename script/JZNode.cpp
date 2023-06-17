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
    Q_ASSERT(prop.isFlow() || prop.isParam() || prop.isSubFlow());

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

int JZNode::addFlowIn()
{
    JZNodePin pin;
    pin.setFlag(Prop_in | Prop_flow);
    return addProp(pin);
}

int JZNode::addFlowOut(QString name)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Prop_out | Prop_flow);
    return addProp(pin);
}

int JZNode::addSubFlowOut(QString name)
{
    JZNodePin pin;
    pin.setName(name);
    pin.setFlag(Prop_out | Prop_subFlow);
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
}

QVector<int> JZNode::propList() const
{
    QVector<int> result;
    for(int i = 0; i < m_propList.size(); i++)
        result.push_back(m_propList[i].id());

    return result;
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

bool JZNode::hasVariable()
{
    return (m_flag & Node_propVariable);
}

void JZNode::setVariable(const QString &)
{

}

QString JZNode::variable() const
{
    return QString();
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

void JZNode::setTypeAny(int id)
{
    prop(id)->setDataType({Type_any});
}

void JZNode::setTypeInt(int id)
{
    prop(id)->setDataType({Type_int,Type_int64});
}

void JZNode::setTypeNumber(int id)
{
    prop(id)->setDataType({Type_bool,Type_int,Type_int64,Type_double});
}

void JZNode::setTypeBool(int id)
{
    prop(id)->setDataType({Type_bool});
}

void JZNode::setTypeString(int id)
{
    prop(id)->setDataType({Type_string});
}


void JZNode::saveToStream(QDataStream &s) const
{
    s << m_id;
    s << m_name;
    s << m_propList;
}

void JZNode::loadFromStream(QDataStream &s)
{
    s >> m_id;
    s >> m_name;
    s >> m_propList;
}

//JZNodeContinue
JZNodeContinue::JZNodeContinue()
{
    m_name = "continue";
    m_type = Node_continue;
    addFlowIn(); 
}

bool JZNodeContinue::compiler(JZNodeCompiler *compiler,QString &error)
{   
    compiler->addContinue();
    return true;
}

//JZNodeBreak
JZNodeBreak::JZNodeBreak()
{
    m_name = "break";
    m_type = Node_break;
    addFlowIn();
}

bool JZNodeBreak::compiler(JZNodeCompiler *compiler,QString &error)
{       
    compiler->addBreak();
    return true;
}


//JZNodeReturn
JZNodeReturn::JZNodeReturn()
{        
    m_name = "return";
    m_type = Node_return;
    addFlowIn();
}

bool JZNodeReturn::compiler(JZNodeCompiler *c,QString &error)
{   
    if(!c->addFlowInput(m_id))
        return false;     

    auto inList = paramInList();
    for(int i = 0; i < inList.size(); i++)
    {
        int id = c->paramId(m_id,inList[i]);
        c->addSetVariable(irId(Reg_Call+i),irId(id));
    }
    c->addReturn();
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
    addFlowOut("continue");
}

int JZNodeSequence::addSequeue()
{
    return addSubFlowOut("Seqeue " + QString::number(subFlowCount() + 1));
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
    addSubFlowOut("loop body");    
    addFlowOut("complete");

    int id_start = addParamIn("First index",Prop_edit | Prop_dispName | Prop_dispValue);
    int id_end = addParamIn("Last index",Prop_edit | Prop_dispName | Prop_dispValue);
    int id_index = addParamOut("index");
    setTypeInt(id_start);
    setTypeInt(id_index);
    setTypeInt(id_end);
}

bool JZNodeFor::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addFlowInput(m_id))
        return false;

    int indexStart = paramIn(0);
    int indexEnd = paramIn(1);
    int indexOut = paramOut(0);

    int id_start = c->paramId(m_id,indexStart);
    int id_end = c->paramId(m_id,indexEnd);
    int id_index = c->paramId(m_id,indexOut);
    c->addSetVariable(irId(id_index),irId(id_start));

    int startPc = c->currentPc() + 1;
    c->addFlowOutput(m_id);
    c->addCompare(irId(id_index),irId(id_end),OP_eq);
    JZNodeIRJmp *jmp_true = new JZNodeIRJmp(OP_je);    
    c->addStatement(JZNodeIRPtr(jmp_true));    
    c->addJumpSubNode(subFlowOut(0));

    int continuePc = c->addExpr(irId(id_index),irId(id_index),irLiteral(1),OP_add);
    JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_jmp);
    jmp->jmpPc = startPc;
    c->addStatement(JZNodeIRPtr(jmp));
    
    int breakPc = c->addJumpNode(flowOut());
    jmp_true->jmpPc = breakPc;
    c->setBreakContinue({breakPc},{continuePc});    

    return true;
}

//JZNodeForEach
JZNodeForEach::JZNodeForEach()
{
    m_name = "foreach";
    m_type = Node_foreach;

    addFlowIn();
    int in = addParamIn("");
    addSubFlowOut("loop body");
    addFlowOut("complete");

    int out1 = addParamOut("key");
    int out2 = addParamOut("value");
    prop(in)->setDataType({Type_list,Type_map});
    setTypeString(out1);
    setTypeAny(out2);
}

JZNodeForEach::~JZNodeForEach()
{

}

bool JZNodeForEach::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addFlowInput(m_id))
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
    c->addCall(irLiteral("string.append"),{className,irLiteral(".iterator")},{itBeginFunc});

    //it = list.first
    c->addCall(itBeginFunc,{list},{it});

    c->addCall(irLiteral("typename"),{it},{itName});
    c->addCall(irLiteral("string.append"),{itName,irLiteral(".next")},{itNextFunc});
    c->addCall(irLiteral("string.append"),{itName,irLiteral(".atEnd")},{itEndFunc});
    c->addCall(irLiteral("string.append"),{itName,irLiteral(".key")},{itKeyFunc});
    c->addCall(irLiteral("string.append"),{itName,irLiteral(".value")},{itValueFunc});

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
    addSubFlowOut("loop body");
    addFlowOut("complete");
    
    int cond = addParamIn("cond");
    setTypeBool(cond);
}


bool JZNodeWhile::compiler(JZNodeCompiler *c,QString &error)
{
    int continuePc = c->currentPc();    
    if(!c->addFlowInput(m_id))
        return false;

    int cond = paramIn(0);
    int id = c->paramId(m_id,cond);
    c->addCompare(irId(id),irLiteral(true),OP_eq);

    JZNodeIRJmp *jmp_true = new JZNodeIRJmp(OP_je);
    JZNodeIRJmp *jmp_false = new JZNodeIRJmp(OP_jmp);
    c->addStatement(JZNodeIRPtr(jmp_true));
    c->addStatement(JZNodeIRPtr(jmp_false));    
    jmp_true->jmpPc = c->addJumpSubNode(subFlowOut(0));           
    jmp_false->jmpPc = c->addJumpNode(flowOut());

    int breakPc = jmp_false->jmpPc;
    c->setBreakContinue({breakPc},{continuePc});

    return true;
}

//JZNodeIf
JZNodeIf::JZNodeIf()
{
    m_name = "if";
}

//JZNodeSwitch
JZNodeSwitch::JZNodeSwitch()
{
    m_name = "switch";
}

//JZNodeBranch
JZNodeBranch::JZNodeBranch()
{
    m_name = "branch";
    m_type = Node_branch;
    addFlowIn();
    addFlowOut("true");
    addFlowOut("false");
    
    addParamIn("cond");

}

bool JZNodeBranch::compiler(JZNodeCompiler *c,QString &error)
{
    int cond = paramIn(0);
    if(!c->addFlowInput(m_id))
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
