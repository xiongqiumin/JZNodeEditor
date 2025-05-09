#include <QDir>
#include <QFile>
#include "JZNodeProgramDumper.h"
#include "JZNodeCompiler.h"
#include "JZParamHelper.h"

JZNodeProgramDumper::JZNodeProgramDumper()
{    
    m_script = nullptr;
    m_project = nullptr;
    m_program = nullptr;
    m_classDefine = nullptr;
    m_function = nullptr;
    m_debug = nullptr;
}

void JZNodeProgramDumper::init(JZProject* project, JZNodeProgram* program)
{
    m_project = project;
    m_program = program;
    m_program->initEnv(&m_env);
}

QString JZNodeProgramDumper::paramDefine(const JZParamDefine* define)
{
    QString line = define->type + " " + define->name + " = ";
    if (define->value.isEmpty())
        line += m_env.defaultValueString(m_env.nameToType(define->type));
    else
        line += define->value;
    return line;
}

bool JZNodeProgramDumper::isFunctionInParam(JZNodeIRParam param)
{
    if (param.isRef())
    {
        auto& func = m_function->define;
        for (int i = 0; i < func.paramIn.size(); i++)
        {
            if (func.paramIn[i].name == param.ref())
                return true;
        }
        return false;
    }
    else
    {
        return false;
    }
}

bool JZNodeProgramDumper::isIrSetReg(int pc, RegSetType regType)
{
    if (pc < 0 || pc >= m_script->statmentList.size())
        return false;

    JZNodeIRParam src, dst;

    if (m_script->statmentList[pc]->type == OP_set)
    {
        JZNodeIRSet* ir_set = dynamic_cast<JZNodeIRSet*>(m_script->statmentList[pc].data());
        src = ir_set->src;
        dst = ir_set->dst;
    }
    else if (m_script->statmentList[pc]->type == OP_clone)
    {
        JZNodeIRClone* ir_set = dynamic_cast<JZNodeIRClone*>(m_script->statmentList[pc].data());
        src = ir_set->src;
        dst = ir_set->dst;
    }
    else
    {
        return false;
    }

    
    if (regType == SrcRegIn)
    {
        return isRegInParam(src);
    }
    else if (regType == SrcRegOut)
    {
        return isRegOutParam(src);
    }
    else if (regType == DstRegIn)
    {
        return isRegInParam(dst);
    }
    else if(regType == DstRegOut)
    {
        return isRegOutParam(dst);
    }

    Q_ASSERT(0);
    return false;
}

bool JZNodeProgramDumper::isRegInParam(JZNodeIRParam param)
{
    if (param.isReg())
    {
        int id = param.id();
        return id >= Reg_CallIn && id < Reg_CallOut;
    }
    else
        return false;
}

bool JZNodeProgramDumper::isRegOutParam(JZNodeIRParam param)
{
    if (param.isReg())
    {
        int id = param.id();
        return id >= Reg_CallOut;
    }
    else
        return false;
}

QString JZNodeProgramDumper::tab(int count)
{
    QString space;
    space.resize(count * 4, ' ');
    return space;
}

void JZNodeProgramDumper::dump(QString dirPath)
{
    QDir dir;
    if (!dir.exists(dirPath) && !dir.mkdir(dirPath))
        return;
    
    m_dirPath = dirPath;
    m_program->initEnv(&m_env);

    auto file_list = m_project->itemList("./", ProjectItem_scriptFile);
    for (int i = 0; i < file_list.size(); i++)
    {
        JZScriptFile *script_file = (JZScriptFile*)file_list[i];
        dumpFile(script_file);
    }
}

void JZNodeProgramDumper::dumpFile(JZScriptFile* script_file)
{
    QFileInfo file_info(script_file->name());
    QString file_name = file_info.baseName();

    QString header;
    QString source;
    header += "#ifndef " + file_name.toUpper() + "_H_\n";
    header += "#define " + file_name.toUpper() + "_H_\n\n";
    header += "#include \"JZRuntime.h\"\n\n";

    source += "#include \"" + file_name + ".h\"\n\n";

    if (file_name == "main")
    {
        QStringList global_list = m_project->globalVariableList();
        for (int global_idx = 0; global_idx < global_list.size(); global_idx++)
        {
            auto global = m_project->globalVariable(global_list[global_idx]);

            source += paramDefine(global)  + ";\n";
            header += "extern " + global->type + " " + global->name + ";\n";
        }
        source += "\n";
        header += "\n";
    }

    QStringList class_list = script_file->classList();
    for (int cls_idx = 0; cls_idx < class_list.size(); cls_idx++)
    {
        JZScriptClassItem* class_item = script_file->getClass(class_list[cls_idx]);
        QString class_define, class_impl;
        dumpClass(class_item->className(), class_define, class_impl);

        header += class_define;
        source += class_impl;
    }

    QStringList function_list = script_file->functionList();
    for (int fun_idx = 0; fun_idx < function_list.size(); fun_idx++)
    {
        const JZFunction* func_item = m_program->function(function_list[fun_idx]);
        QString func_define, func_impl;
        dumpFunction(func_item, func_define, func_impl);

        header += func_define + ";\n";
        source += func_impl + "\n\n";
    }

    header += "\n#endif\n";

    QString save_path = m_dirPath + "/" + file_info.baseName();
    QFile h_file(save_path + ".h");
    if (h_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        h_file.write(header.toUtf8());
        h_file.close();
    }

   QFile cpp_file(save_path + ".cpp");
    if (cpp_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        cpp_file.write(source.toUtf8());
        cpp_file.close();
    }
}

int JZNodeProgramDumper::irParamType(const JZNodeIRParam& param)
{
    int type = Type_none;

    if (param.isLiteral())
        type = JZNodeType::variantType(param.literal());
    else if (param.isThis())
        type = JZNodeType::pointerType(m_classDefine->id);
    else if (param.isRef())
    {
        auto coor = JZParamHelper::splitMember(param.ref());
        if (coor.baseName == "this")
            type = m_classDefine->id;
        else
        {
            auto local_def = m_debug->localParam(coor.baseName);
            if (local_def)
                type = local_def->dataType;
            else
            {
                auto global_def = m_project->globalVariable(coor.baseName);
                type = m_env.nameToType(global_def->type);
            }
        }

        if (coor.memberList.size() != 0)
        {
            auto obj_def = m_env.meta(type);
            Q_ASSERT(obj_def);
                
            auto param_def = JZParamHelper::memberDefine(obj_def, coor.memberList);
            type = m_env.nameToType(param_def->type);
        }
    }
    else if (param.isNodeId())
    {
        auto gemo = JZNodeGemo::fromParamId(param.id());
        auto node_info = m_debug->nodeParam(param.id());
        Q_ASSERT(node_info);

        type = node_info->dataType;
    }
    else if (param.isStack())
    {        
        type = m_debug->stackType.value(param.id(), Type_none);
    }

    Q_ASSERT(type != Type_none);
    return type;
}

void JZNodeProgramDumper::dumpClass(QString class_name, QString& def, QString& impl)
{
    auto cls = m_env.meta(class_name);
    m_classDefine = cls;

    QString name = cls->className;
    QString super = cls->superName;

    auto class_item = m_project->getClass(class_name);
    JZUiItem* ui = class_item->ui();

    QString header;
    QString source;

    header += "class " + name;
    if (!super.isEmpty())
        header += " : public " + super;
    header += "\n{\n";

    header += "public:\n";
    header += tab(1) + name + "();\n";
    header += tab(1) + "virtual ~" + name + "();\n\n";

    //function
    auto function_list = cls->functions;
    for (int i = 0; i < function_list.size(); i++)
    {
        auto func = m_program->function(function_list[i].fullName());
        QString func_def, func_impl;
        dumpFunction(func, func_def, func_impl);

        header += tab(1) + func_def + ";\n";
        source += func_impl + "\n\n";
    }
    
    auto member_list = class_item->memberVariableList(true);
    for (int i = 0; i < member_list.size(); i++)
    {
        auto member = class_item->memberVariable(member_list[i], true);
        header += tab(1) + member->type + " " + member->name + ";\n";
    }

    header += "};\n\n";

    def = header;
    impl = source;

    m_classDefine = nullptr;
}

void JZNodeProgramDumper::dumpFunction(const JZFunction* func_impl, QString& def, QString& impl)
{
    QString source;
    QString header;

    m_function = func_impl;
    m_script = m_program->script(func_impl->path);    
    m_debug = m_script->functionDebug(func_impl->fullName());
    auto& opList = m_script->statmentList;

    QString declare = functionDeclare(func_impl);
    if (m_classDefine)
    {
        int idx = declare.indexOf(' ');
        declare.insert(idx + 1, m_classDefine->className + "::");
    }

    source += declare + "\n{\n";
    m_jumpList.clear();
    source += tab(1) + "bool Reg_Cmp = false;\n";

    QString space = QString().leftJustified(12,' ');
    QStringList lines;
    for (int i = func_impl->addr; i < func_impl->addrEnd; i++)
    {
        QString line;
        line += irToString(opList[i].data());
        lines.push_back(line);
    }

    //函数加{}
    for (int i = func_impl->addr; i < func_impl->addrEnd; i++)
    {
        if (opList[i].data()->type != OP_call)
            continue;

        int set_reg_in = -1;
        for (int j = i - 1; j >= 0; j--)
        {
            if (isIrSetReg(j, DstRegIn))
                set_reg_in = j;
            else
                break;
        }

        if(set_reg_in != -1)
            lines[set_reg_in] = space + "{\n" + lines[set_reg_in];

        int set_reg_out = (set_reg_in == -1)? -1:i;
        for (int j = i + 1; j < func_impl->addrEnd; j++)
        {
            if (isIrSetReg(j, SrcRegOut))
                set_reg_out = j;
            else
                break;
        }
        if(set_reg_out != -1)
            lines[set_reg_out] = lines[set_reg_out] + "\n" + space + "}";
    }

    //return 返回值
    for (int i = func_impl->addr; i < func_impl->addrEnd; i++)
    {
        if (opList[i].data()->type != OP_return)
            continue;

        if (isIrSetReg(i - 1, DstRegOut))
        {
            lines[i - 1] = space + "{\n" + lines[i - 1];
            lines[i] = lines[i] + "\n" + space + "}";
        }
    }

    //处理跳转
    std::sort(m_jumpList.begin(), m_jumpList.end());
    for (int i = m_jumpList.size() - 1; i >= 0; i--)
    {
        int addr = m_jumpList[i];
        lines.insert(addr - func_impl->addr, "Line" + QString::number(addr) + ":");
    }

    source += lines.join("\n");
    source += "\n}";

    header += functionDeclare(func_impl);

    def = header;
    impl = source;
}

QString JZNodeProgramDumper::toString(JZNodeIRParam param)
{
    if (param.type == JZNodeIRParam::Literal)
    {
        QVariant value = param.literal();
        auto var_type = JZNodeType::variantType(value);
        if (var_type == Type_string)
        {
            return "R\"(" + value.toString() + ")\"";
        }
        else if (var_type == Type_class)
        {
            return m_env.typeToName(var_type) + "()";
        }
        else
        {
            return JZNodeType::debugString(value);
        }
    }
    else if(param.type == JZNodeIRParam::Reference)
        return param.ref();
    else if(param.type == JZNodeIRParam::This)
        return "this";
    else
    {
        QString name = JZNodeCompiler::paramName(param.id());
        name.replace(".", "_");
        return name;
    }
}

QString JZNodeProgramDumper::irToString(JZNodeIR *op)
{    
    QString line;

    switch (op->type)
    {
    case OP_nodeEnter:
    {
        JZNodeIRNodeEnter *ir_node = (JZNodeIRNodeEnter*)op;
        line += "//node" + QString::number(ir_node->id);
        break;
    }
    case OP_nop:
    {
        line += "//nop";
        break;
    }
    case OP_alloc:
    {
        JZNodeIRAlloc *ir_alloc = (JZNodeIRAlloc*)op;

        QString alloc = m_env.typeToName(ir_alloc->dataType);
        if (ir_alloc->allocType == JZNodeIRAlloc::Heap || ir_alloc->allocType == JZNodeIRAlloc::Stack)
            line += alloc + " " + toString(ir_alloc->dst);
        else
            line += alloc + " " + toString(ir_alloc->dst);

        if (JZNodeType::isBase(ir_alloc->dataType))
        {
            line += " = " + m_env.defaultValueString(ir_alloc->dataType);
        }
        else 
        { 
            if (JZNodeType::isPointer(ir_alloc->dataType))
                line += " = nullptr";
        }
        line += ";";
        if (isFunctionInParam(ir_alloc->dst))
            line = "// " + line;
        break;
    }
    case OP_reference:
    {
        JZNodeIRReference* ir_ref = (JZNodeIRReference*)op;
        int data_type = irParamType(ir_ref->orig);

        QString alloc = m_env.typeToName(data_type);
        line += alloc + " &" + toString(ir_ref->ref)  + " = " + toString(ir_ref->orig) + ";";
        break;
    }
    case OP_clearReg:
        line += "// clear reg";
        break;
    case OP_set:
    {
        JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
        line += toString(ir_set->dst) + " = " + toString(ir_set->src) + ";";
        if (isRegInParam(ir_set->dst) || isRegOutParam(ir_set->dst)) //调用函数
        {
            line = "auto&& " + line;
        }
        if (isRegInParam(ir_set->src))  //函数入参
        {
            line = "// " + line;
        }
        break;
    }
    case OP_clone:
    {
        JZNodeIRClone* ir_set = (JZNodeIRClone*)op;   
        line += toString(ir_set->dst) + " = " + toString(ir_set->src) + "; //clone";
        if (isRegInParam(ir_set->dst) || isRegOutParam(ir_set->dst)) //调用函数
        {
            line = "auto&& " + line;
        }
        if (isRegInParam(ir_set->src)) //函数入参
        {
            line = "// " + line;
        }
        break;
    }
    case OP_convert:
    {
        JZNodeIRConvert *ir_cvt = (JZNodeIRConvert*)op;
        if(!JZNodeType::isPointer(irParamType(ir_cvt->src)) && JZNodeType::isPointer(ir_cvt->dstType))
            line += toString(ir_cvt->dst) + " = &" + toString(ir_cvt->src) + ";";
        else
            line += toString(ir_cvt->dst) + " = (" + m_env.typeToName(ir_cvt->dstType) + ")" + toString(ir_cvt->src) + ";";
        break;
    }
    case OP_call:
    {
        JZNodeIRCall *ir_call = (JZNodeIRCall *)op;
        line += dealCall(ir_call->function);

        if (isIrSetReg(op->pc + 1, SrcRegOut))
        {
            line = "auto&& Reg_CallOut_0 = " + line;
        }
        break;
    }
    case OP_return:
    {
        if (isIrSetReg(op->pc - 1, DstRegOut))
            line = "return Reg_CallOut_0;";
        else
            line += "return;";
        break;
    }
    case OP_add:
    case OP_sub:
    case OP_mul:
    case OP_div:
    case OP_mod:
    case OP_eq:
    case OP_ne:
    case OP_le:
    case OP_ge:
    case OP_lt:
    case OP_gt:
    case OP_and:
    case OP_or:
    case OP_bitand:
    case OP_bitor:
    case OP_bitxor:
    {
        JZNodeIRExpr *ir_expr = (JZNodeIRExpr *)op;
        QString c = toString(ir_expr->dst);
        QString a = toString(ir_expr->src1);
        QString b = toString(ir_expr->src2);
        line += c + " = " + a + " " + JZNodeType::opName(op->type) + " " + b + ";";
        break;
    }
    case OP_neg:
    case OP_not:
    case OP_bitreverse:
    {
        JZNodeIRExpr *ir_expr = (JZNodeIRExpr *)op;
        QString c = toString(ir_expr->dst);
        QString a = toString(ir_expr->src1);
        line += c + " = " + JZNodeType::opName(op->type) + a + ";";
        break;
    }
    case OP_jmp:
    case OP_jne:
    case OP_je:
    {
        JZNodeIRJmp *ir_jmp = (JZNodeIRJmp *)op;
        if (op->type == OP_jmp)
            line += "JMP(" + QString::number(ir_jmp->jmpPc) + ");";
        else if (op->type == OP_je)
            line += "JE(" + QString::number(ir_jmp->jmpPc) + ");";
        else
            line += "JNE(" + QString::number(ir_jmp->jmpPc) + ");";
        
        if (!m_jumpList.contains(ir_jmp->jmpPc))
            m_jumpList.push_back(ir_jmp->jmpPc);
        break;
    }
    case OP_assert:
    {
        JZNodeIRAssert *ir_assert = (JZNodeIRAssert *)op;
        line += "assert(" + toString(ir_assert->tips) + ")";
        break;
    }
    case OP_try:
    {
        break;
    }
    case OP_throw:
    {
        break;
    }
    default:
        Q_ASSERT(0);
        break;
    }

    if (!op->memo.isEmpty())
    {
        line = line.leftJustified(12);
        if (line.startsWith("//"))
            line += op->memo;
        else
            line += " //" + op->memo;
    }
    line = QString::asprintf("/*%04d*/", op->pc) + "    " + line;
    return line;
}    

QString JZNodeProgramDumper::functionDeclare(const JZFunction* func)
{
    auto& define = func->define;

    QString returnType = "void";
    if (define.paramOut.size() > 0)
        returnType = define.paramOut[0].type;

    QString line = returnType + " " + define.name + "(";
    QStringList param_str;
    int start = define.isMemberFunction() ? 1 : 0;
    for (int i = start; i < define.paramIn.size(); i++)
    {
        auto p = define.paramIn[i];
        param_str.push_back(p.type + " " + p.name);
    }
    return line + param_str.join(",") + ")";
}

QString JZNodeProgramDumper::dealCall(QString function)
{
    auto func_def = m_env.function(function);
    QString c_func = func_def->name;
    if (func_def->className.startsWith("QList<"))
    {
        if (func_def->name == "set")
            c_func = "replace";
        else if (func_def->name == "get")
            c_func = "at";
    }

    QString line;
    int reg_idx = 0;
    if (func_def->isMemberFunction())
    {
        line = JZNodeCompiler::paramName(Reg_CallIn + reg_idx++) + "->";
    }
    QStringList param_in_list;
    for(int i = reg_idx; i < func_def->paramIn.size(); i++)
        param_in_list << JZNodeCompiler::paramName(Reg_CallIn + i);
    line = line + c_func + "(" + param_in_list.join(",") + ");";

    return line;
}