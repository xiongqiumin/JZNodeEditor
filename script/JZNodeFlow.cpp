#include "JZNodeCompiler.h"
#include "JZNodeFlow.h"

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
    c->addNodeEnter(m_id);
    c->addNop();
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
    JZNode* parent = c->continueParentNode(m_id);
    if (!parent)
    {
        error = "continue 需要在for,foreach,while中使用";
        return false;
    }

    c->addNodeEnter(m_id);
    c->addContinue(m_id);
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
    JZNode* parent = c->breakParentNode(m_id);
    if (!parent)
    {
        error = "break 需要在for,foreach,while,switch中使用";
        return false;
    }
    c->addNodeEnter(m_id);
    c->addBreak(m_id);
    return true;
}


//JZNodeReturn
JZNodeReturn::JZNodeReturn()
{        
    m_name = "return";
    m_type = Node_return;
    addFlowIn();
}

bool JZNodeReturn::updateNode(QString &error)
{
    auto env = environment();
    auto inList = paramInList();
    QList<JZParamDefine> ret_list;
    if(m_file->itemType() == ProjectItem_scriptItem)
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
        int in = addParamIn(def->paramOut[i].name);
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

//JZNodeSequence
JZNodeSequence::JZNodeSequence()
{
    m_name = "sequence";
    m_type = Node_sequence;
    addFlowIn();
    addFlowOut("complete");

    addSequeue();
    addSequeue();
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
    return addSubFlowOut("Seqeue " + QString::number(subFlowCount() + 1));
}

void JZNodeSequence::removeSequeue(int id)
{
    removePin(id);
    updateSeqName();
}


bool JZNodeSequence::compiler(JZNodeCompiler *c,QString &error)
{
    c->addNodeEnter(m_id);
    //设置continue, 最后一个是跳出
    auto list = subFlowList();
    for(int i = 0; i < list.size(); i++)
    {
        JZNode* sub_node = c->nextFlowNode(this, subFlowOut(i));
        if (sub_node)
        {
            QList<JZNodeIRPtr> ir_list;
            c->buildSubControlFlow(sub_node, ir_list);
            c->appendStatementList(ir_list);
        }
    }
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

    int id_start = addParamIn("Index");
    int id_step = addParamIn("Step");
    int id_end = addParamIn("End index");    
    int id_index = addParamOut("Index");
    setPinTypeInt(id_start);
    setPinTypeInt(id_step);
    setPinTypeInt(id_index);
    setPinTypeInt(id_end);    

    setPinValue(id_start, "0");
    setPinValue(id_step, "1");
    setPinValue(id_end, "1");    

    m_condOp = OP_lt;
    m_condOpList.push_back(OP_lt);
    m_condOpList.push_back(OP_le);
    m_condOpList.push_back(OP_gt);
    m_condOpList.push_back(OP_ge);
    m_condOpList.push_back(OP_eq);
    m_condOpList.push_back(OP_ne);
}

bool JZNodeFor::compiler(JZNodeCompiler *c,QString &error)
{
    c->setAutoaddNodeEnter(m_id, false);
    if (!c->addFlowInput(m_id, error))    
        return false;        

    int indexStart = paramIn(0);
    int indexStep = paramIn(1);
    int indexEnd = paramIn(2);
    int indexOut = paramOut(0);        
    JZNodeIRType op = (JZNodeIRType)m_condOp;

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
    int start = c->addNodeEnter(m_id);
    c->addCompare(irId(id_index), irId(id_end), op);
    JZNodeIRJmp *jmp_cond_break = c->addJmp(OP_jne); 
    c->lastStatment()->memo = "body";    
    c->addFlowOutput(m_id);

    //body
    JZNode* sub_node = c->nextFlowNode(this, subFlowOut(0));
    if (sub_node)
    {
        QList<JZNodeIRPtr> ir_list;
        c->buildSubControlFlow(sub_node, ir_list);
        c->appendStatementList(ir_list);
    }

    //continue
    int continue_pc = c->addExpr(irId(id_index), irId(id_index), irId(id_step), OP_add);
    c->addSetVariable(irId(id_out_index), irId(id_index));
    JZNodeIRJmp *jmp_to_start = c->addJmp(OP_jmp);
    jmp_to_start->jmpPc = start;
        
    int break_pc = c->addNop();        
    jmp_cond_break->jmpPc = break_pc;
    c->setBreakContinue(break_pc, continue_pc);

    return true;
}

void JZNodeFor::saveToStream(QDataStream& s) const
{
    JZNode::saveToStream(s);
    s << m_condOp;
}

void JZNodeFor::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);  
    s >> m_condOp;
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

JZNodeIRType JZNodeFor::op()
{
    return m_condOp;
}

void JZNodeFor::setOp(JZNodeIRType op)
{    
    Q_ASSERT(m_condOpList.contains(op));
    m_condOp = op;
}

//JZNodeForEach
JZNodeForEach::JZNodeForEach()
{
    m_name = "foreach";
    m_type = Node_foreach;

    addFlowIn();
    int in = addParamIn("");
    setPinType(paramIn(0), { "arg*" });
    addSubFlowOut("loop body");
    addFlowOut("complete");

    int out1 = addParamOut("key");
    int out2 = addParamOut("value");
}

JZNodeForEach::~JZNodeForEach()
{

}

bool JZNodeForEach::getInputType(int &class_type,int &from_id, QString& error)
{
    QList<int> list = m_file->getConnectInput(m_id, paramIn(0));
    if (list.size() != 1)
    {
        error = "no input";
        return false;
    }

    auto line = m_file->getConnect(list[0]);
    QStringList pin_types = m_file->getPin(line->from)->dataType();
    if (pin_types.size() != 1)
    {
        error = "not support input";
        return false;
    }

    auto env = environment();
    class_type = env->nameToType(pin_types[0]);
    class_type = JZNodeType::baseType(class_type);
    from_id = line->from.paramId();
    return true;
}

void JZNodeForEach::onPinLinked(int pin_id)
{
    update();
}

void JZNodeForEach::onPinUnlinked(int pin_id)
{
    update();
}

bool JZNodeForEach::updateNode(QString& error)
{
    int in_type, from_id;
    if (!getInputType(in_type, from_id, error))
        return false;

    auto env = environment();
    if (env->isListType(in_type))
    {
        int value_type = 0;
        if (!env->listValueType(in_type, value_type))
            return false;

        setPinType(paramOut(0), { env->typeToName(Type_int) });
        setPinType(paramOut(1), { env->typeToName(value_type) });
        return true;
    }
    else if (env->isMapType(in_type))
    {
        int key_type = 0;
        int value_type = 0;
        if (env->mapKeyValueType(in_type, key_type, value_type))
            return false;

        setPinType(paramOut(0), { env->typeToName(key_type) });
        setPinType(paramOut(1), { env->typeToName(value_type) });
        return true;
    }
    else
    {
        error = "only support list and map";
        return false;
    }
}

bool JZNodeForEach::compiler(JZNodeCompiler *c,QString &error)
{
    int class_type, from_id;
    if(!getInputType(class_type, from_id, error))
        return false;

    if (!c->addFlowInput(m_id, error))
        return false;
    
    int class_id = c->paramId(m_id, paramIn(0));
    auto obj_inst = environment()->objectManager();
    auto meta = obj_inst->meta(class_type);    

    JZNodeIRParam class_ptr = irId(class_id);
    JZNodeIRParam itKey = irId(c->paramId(m_id, paramOut(0)));
    JZNodeIRParam itValue = irId(c->paramId(m_id, paramOut(1)));
    JZNodeIRJmp* jmp_break;

    int continuePc = 0;
    if (environment()->isListType(class_type))
    {
        JZNodeIRParam itEnd = irId(c->addAllocStack(Type_int));

        auto* func_size = meta->function("size");
        auto* func_get = meta->function("get");

        c->addSetVariable(itKey, irLiteral(0));
        c->addCall(func_size, { class_ptr }, { itEnd });

        //it < size
        int startPc = c->nextPc();
        c->addCompare(itKey, itEnd, OP_lt);
        jmp_break = c->addJmp(OP_jne);
        
        c->addCallConvert(func_get, { class_ptr, itKey }, { itValue });
        c->addFlowOutput(m_id);

        //body
        JZNode* sub_node = c->nextFlowNode(this, subFlowOut(0));
        if (sub_node)
        {
            QList<JZNodeIRPtr> ir_list;
            c->buildSubControlFlow(sub_node, ir_list);
            c->appendStatementList(ir_list);
        }

        //continue
        continuePc = c->nextPc();
        c->addExpr(itKey, itKey, irLiteral(1), OP_add);
        JZNodeIRJmp* jmp = c->addJmp(OP_jmp);
        jmp->jmpPc = startPc;
    }
    else if (environment()->isMapType(class_type))
    {
        auto* it_begin = meta->function("begin");
        auto* it_end = meta->function("end");

        int it_type;
        environment()->mapIteratorType(class_type, it_type);

        JZNodeIRParam it = irId(c->addAllocStack(it_type));
        JZNodeIRParam itEnd = irId(c->addAllocStack(it_type));

        c->addCall(it_begin, { class_ptr }, { it });
        c->addCall(it_end, { class_ptr }, { itEnd });

        auto* it_define = meta->function("iterator");
        auto it_meta = obj_inst->meta(it_define->paramOut[0].type);
        auto itNextFunc = it_meta->function("next");
        auto itKeyFunc = it_meta->function("key");
        auto itValueFunc = it_meta->function("value");

        //while(it != it.end()
        int startPc = c->nextPc();
        c->addCompare(it, itEnd, OP_eq);
        jmp_break = c->addJmp(OP_jne);

        c->addCallConvert(itKeyFunc, { it }, { itKey });
        c->addCallConvert(itValueFunc, { it }, { itValue });
        c->addFlowOutput(m_id);

        //body
        JZNode* sub_node = c->nextFlowNode(this, subFlowOut(0));
        if (sub_node)
        {
            QList<JZNodeIRPtr> ir_list;
            c->buildSubControlFlow(sub_node, ir_list);
            c->appendStatementList(ir_list);
        }

        //continue
        continuePc = c->nextPc();
        c->addCallConvert(itNextFunc, { it }, { });

        JZNodeIRJmp *jmp = c->addJmp(OP_jmp);
        jmp->jmpPc = startPc;
    }
    else
    {
        Q_ASSERT(0);
    }

    int breakPc = c->addNop();
    jmp_break->jmpPc = breakPc;
    c->setBreakContinue( breakPc , continuePc);

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

    JZNodeIRJmp * jmp_false = c->addJmp(OP_jne);

    //body
    JZNode* sub_node = c->nextFlowNode(this, subFlowOut(0));
    if (sub_node)
    {
        QList<JZNodeIRPtr> ir_list;
        c->buildSubControlFlow(sub_node, ir_list);
        c->appendStatementList(ir_list);
    }
    JZNodeIRJmp* jmp = c->addJmp(OP_jmp);
    jmp->jmpPc = continuePc;

    int breakPc = c->addNop();
    jmp_false->jmpPc = breakPc;
    
    c->setBreakContinue( breakPc, continuePc );
    return true;
}

//JZNodeIf
JZNodeIf::JZNodeIf()
{
    m_type = Node_if;
    m_name = "if";
    addFlowIn();
    addFlowOut("complete");
    addCondPin();    
}

int JZNodeIf::addCondPin()
{
    int in = addParamIn("cond");
    setPinTypeBool(in);

    int id = addSubFlowOut("cond");    
    return in;
}

bool JZNodeIf::hasElse()
{
    bool isElse = subFlowCount() > paramInCount();
    return isElse;
}

void JZNodeIf::addElsePin()
{
    addSubFlowOut("else"); 
}

int JZNodeIf::condCount()
{
    return paramInCount();
}

void JZNodeIf::removeCond(int id)
{
    int index = paramInList().indexOf(id);
    Q_ASSERT(index >= 0);

    int flow_id = paramInList()[index];
    int in_id = subFlowList()[index];
    removePin(in_id);
    removePin(flow_id);
}

void JZNodeIf::removeElse()
{
    if (!hasElse())
        return;

    int id = subFlowList().back();
    removePin(id);
}

bool JZNodeIf::compiler(JZNodeCompiler *c, QString &error) 
{
    QList<int> breakPc,continuePc;
    bool isElse = subFlowCount() > paramInCount();
    
    JZNodeIRJmp *last_jmp = nullptr;
    QList<int> inList = paramInList();
    QList<JZNodeIRJmp*> jump_end_list;
    for (int i = 0; i < inList.size(); i++)
    {
        int nextPc = c->nextPc();
        if (!c->addFlowInput(m_id, { inList[i] }, error))
            return false;

        if (last_jmp)
            last_jmp->jmpPc = nextPc;

        int cond = c->paramId(m_id, inList[i]);
        c->addCompare(irId(cond), irLiteral(true), OP_eq);

        JZNodeIRJmp *jmp = c->addJmp(OP_jne);
        last_jmp = jmp;

        JZNode* sub_node = c->nextFlowNode(this, subFlowOut(i));
        if (sub_node)
        {
            QList<JZNodeIRPtr> ir_list;
            c->buildSubControlFlow(sub_node, ir_list);
            c->appendStatementList(ir_list);
        }

        JZNodeIRJmp* jmp_end = c->addJmp(OP_jmp);
        jump_end_list << jmp_end;
    }    
    if (isElse)
    {
        if (last_jmp)
            last_jmp->jmpPc = c->nextPc();

        JZNode* sub_node = c->nextFlowNode(this, subFlowOut(inList.size()));
        if (sub_node)
        {
            QList<JZNodeIRPtr> ir_list;
            c->buildSubControlFlow(sub_node, ir_list);
            c->appendStatementList(ir_list);
        }
        else
        {
            c->addNop();
        }
        JZNodeIRJmp* jmp_end = c->addJmp(OP_jmp);
        jump_end_list << jmp_end;
    }    
    else
    {
        jump_end_list << last_jmp;
    }

    int pc = c->addNop();
    for (int i = 0; i < jump_end_list.size(); i++)
        jump_end_list[i]->jmpPc = pc;

    return true;
}

//JZNodeSwitch
JZNodeSwitch::JZNodeSwitch()
{
    m_type = Node_switch;
    m_name = "switch";
    m_caseType << JZNodeType::typeName(Type_int) << JZNodeType::typeName(Type_string);

    addFlowIn();
    addFlowOut("complete");
    int in = addParamIn("value");    
    setPinType(in, m_caseType);

    addParamOut("cond");
    addCase();
}

int JZNodeSwitch::addCase()
{
    int sub_id = addSubFlowOut("case");
    setPinType(sub_id, m_caseType);
    return sub_id;
}

int JZNodeSwitch::addDefault()
{
    return addSubFlowOut("default");    
}

void JZNodeSwitch::removeCase(int id)
{
    removePin(id);
}

void JZNodeSwitch::removeDefault()
{
    int id = subFlowList().back();
    removePin(id);    
}

bool JZNodeSwitch::hasDefault()
{
    bool has_default = paramOutCount() < subFlowCount();
    return has_default;
}

int JZNodeSwitch::caseCount()
{
    auto list = subFlowList();
    int id = list.back();
    bool isDefault = pin(id)->name() == "default";
    if (isDefault)
        return list.size() - 1;
    else
        return list.size();
}

void JZNodeSwitch::clearCaseAndDefault()
{
    auto sub_list = subFlowList();
    for (int i = 0; i < sub_list.size(); i++)
        removePin(sub_list[i]);
}

void JZNodeSwitch::setCaseValue(int index, const QString &v)
{
    Q_ASSERT(index < caseCount());
    pin(subFlowOut(index))->setValue(v);
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

    bool isDefault = false;
    int case_count = sub_flow_list.size();
    if (pin(sub_flow_list.back())->name() == "default")
    {
        case_count--;
        isDefault = true;
    }
    int in_type = c->pinType(m_id, paramIn(0));

    QList<JZNodeIRJmp*> jump_end_list;
    JZNodeIRJmp *pre_jmp = nullptr;
    for (case_idx = 0; case_idx < case_count; case_idx++)
    {
        QString out_value_str = pin(sub_flow_list[case_idx])->value();
        QVariant value = env->initValue(in_type, out_value_str);

        int jmp_cmp = c->addCompare(irId(in_id), irLiteral(value), OP_eq);
        JZNodeIRJmp *jmp_false = c->addJmp(OP_jne);

        JZNode* sub_node = c->nextFlowNode(this, subFlowOut(case_idx));
        if (sub_node)
        {
            QList<JZNodeIRPtr> ir_list;
            c->buildSubControlFlow(sub_node, ir_list);
            c->appendStatementList(ir_list);
        }

        JZNodeIRJmp* jmp_end = c->addJmp(OP_jmp);
        jump_end_list << jmp_end;

        if (pre_jmp)
            pre_jmp->jmpPc = jmp_cmp;
        pre_jmp = jmp_false;
    }
    if (isDefault)
    {
        if (pre_jmp)
            pre_jmp->jmpPc = c->nextPc();

        JZNode* sub_node = c->nextFlowNode(this, subFlowOut(case_count));
        if (sub_node)
        {
            QList<JZNodeIRPtr> ir_list;
            c->buildSubControlFlow(sub_node, ir_list);
            c->appendStatementList(ir_list);
        }
        else
        {
            c->addNop();
        }

        JZNodeIRJmp* jmp_end = c->addJmp(OP_jmp);
        jump_end_list << jmp_end;
    }
    else
    {
        jump_end_list << pre_jmp;
    }

    int out_pc = c->addNop();
    for (int i = 0; i < jump_end_list.size(); i++)
        jump_end_list[i]->jmpPc = out_pc;

    c->setBreakContinue(out_pc ,-1);
    return true;
}

//JZNodeTryCatch
JZNodeTryCatch::JZNodeTryCatch()
{
    m_name = "tryCatch";
    m_type = Node_tryCatch;
    addFlowIn();
    addFlowOut("complete");
    addSubFlowOut("try");
    addSubFlowOut("catch");
    int exp_pin = addParamOut("exception");
    setPinTypeString(exp_pin);
}

bool JZNodeTryCatch::compiler(JZNodeCompiler *c,QString &error)
{
    int cond = paramIn(0);
    if(!c->addFlowInput(m_id,error))
        return false;

    JZNodeIRTry* ir_try = new JZNodeIRTry();
    ir_try->catchType = JZNodeIRTry::InTry;
    ir_try->catchPc = 0;
    c->addStatement(JZNodeIRPtr(ir_try));

    QList<JZNodeIRJmp*> jump_end_list;
    JZNode* try_node = c->nextFlowNode(this, subFlowOut(0));
    if (try_node)
    {
        QList<JZNodeIRPtr> ir_list;
        c->buildSubControlFlow(try_node, ir_list);
        c->appendStatementList(ir_list);
    }
    //end try
    JZNodeIRTry* ir_try_end = new JZNodeIRTry();
    ir_try_end->catchType = JZNodeIRTry::OutTry;
    c->addStatement(JZNodeIRPtr(ir_try_end));
    jump_end_list << c->addJmp(OP_jmp);
    
    int catch_pc = c->addNop();
    c->addFlowOutput(m_id);
    ir_try->catchPc = catch_pc;
    ir_try->irExcep = irId(c->paramId(m_id, paramOut(0)));
    JZNode* catch_node = c->nextFlowNode(this, subFlowOut(1));
    if (catch_node)
    {
        QList<JZNodeIRPtr> ir_list;
        c->buildSubControlFlow(catch_node, ir_list);
        c->appendStatementList(ir_list);
    }
    jump_end_list << c->addJmp(OP_jmp);
    
    int out_pc = c->addNop();
    for (int i = 0; i < jump_end_list.size(); i++)
        jump_end_list[i]->jmpPc = out_pc;

    return true;
}

//JZNodeThrow
JZNodeThrow::JZNodeThrow()
{
    m_name = "throw";
    m_type = Node_tryCatch;
    addFlowIn();

    int cond = addParamIn("error");
    setPinTypeString(cond);
}

bool JZNodeThrow::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addFlowInput(m_id,error))
        return false;

    JZNodeIRThrow* ir = new JZNodeIRThrow();
    ir->exception = irId(c->paramId(m_id, paramIn(0)));
    c->addStatement(JZNodeIRPtr(ir));

    return true;
}

//JZNodeAssert
JZNodeAssert::JZNodeAssert()
{
    m_name = "assert";
    m_type = Node_assert;
    addFlowIn();
    addFlowOut();    

    int cond = addParamIn("cond");
    setPinTypeBool(cond);

    int tips = addParamIn("tips");
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

//JZNodeMainLoop
JZNodeMainLoop::JZNodeMainLoop()
{        
    m_type = Node_mainLoop;
    m_name = "MainLoop";

    addFlowIn();
    int in = addParamIn("window");
    setPinType(in, { "QWidget" });
}

JZNodeMainLoop::~JZNodeMainLoop()
{    
}

bool JZNodeMainLoop::compiler(JZNodeCompiler *c, QString &error)
{
    if(!c->addFlowInput(m_id,error))
        return false;    
    
    int id = c->paramId(m_id, paramIn(0));
    c->addAlloc(JZNodeIRAlloc::Heap, "__mainwindow__", Type_widget);

    JZNodeIRSet *ir_set = new JZNodeIRSet();
    ir_set->dst = irRef("__mainwindow__");
    ir_set->src = irId(id);
    c->addStatement(JZNodeIRPtr(ir_set));

    return true;    
}