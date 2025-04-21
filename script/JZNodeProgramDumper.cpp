#include <QDir>
#include <QFile>
#include "JZNodeProgramDumper.h"
#include "JZNodeCompiler.h"    

JZNodeProgramDumper::JZNodeProgramDumper()
{    
    m_program = nullptr;
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
        auto &func = m_script->function();
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
    if (pc < 0 || pc >= m_scriptImpl->statmentList.size())
        return false;

    if (m_scriptImpl->statmentList[pc]->type != OP_set)
        return false;

    JZNodeIRSet* ir_set = dynamic_cast<JZNodeIRSet*>(m_scriptImpl->statmentList[pc].data());
    if (regType == SrcRegIn)
    {
        return isRegInParam(ir_set->src);
    }
    else if (regType == SrcRegOut)
    {
        return isRegOutParam(ir_set->src);
    }
    else if (regType == DstRegIn)
    {
        return isRegInParam(ir_set->dst);
    }
    else if(regType == DstRegOut)
    {
        return isRegOutParam(ir_set->dst);
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

    source += "#include \"JZRuntime.h\"\n";
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
        dumpClass(class_item, class_define, class_impl);

        header += class_define;
        source += class_impl;
    }

    QStringList function_list = script_file->functionList();
    for (int fun_idx = 0; fun_idx < function_list.size(); fun_idx++)
    {
        JZScriptItem* func_item = script_file->getFunction(function_list[fun_idx]);
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


void JZNodeProgramDumper::dumpClass(JZScriptClassItem* class_item, QString& def, QString& impl)
{
    auto cls = m_env.meta(class_item->className());

    QString name = cls->className;
    QString super = cls->superName;
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

    auto function_list = class_item->memberFunctionList();
    for (int i = 0; i < function_list.size(); i++)
    {
        auto func = class_item->memberFunction(function_list[i]);
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

    header += "};";

    def = header;
    impl = source;
}

void JZNodeProgramDumper::dumpFunction(JZScriptItem* func_item, QString& def, QString& impl)
{
    QString source;
    QString header;

    auto func_impl = m_program->function(func_item->function().fullName());

    m_script = func_item;
    m_scriptImpl = m_program->script(func_impl->path);
    
    auto& opList = m_scriptImpl->statmentList;

    source += functionDeclare(func_impl) + "\n{\n";
    m_jumpList.clear();

    source += tab(1) + "bool Reg_Cmp = false;\n";

    QString space = QString().leftJustified(8,' ');
    QStringList lines;
    for (int i = func_impl->addr; i < func_impl->addrEnd; i++)
    {
        QString line;

        //函数调用前加{
        if (!isIrSetReg(i - 1, DstRegIn) && isIrSetReg(i, DstRegIn))
            line += space + "{\n";

        line += irToString(opList[i].data());

        //函数调用后加}
        if ((opList[i]->type == OP_call && !isIrSetReg(i - 1, DstRegIn) && !isIrSetReg(i + 1, SrcRegOut))
            || (isIrSetReg(i, SrcRegOut)))
        {
            line += space + "\n}";
        }

        lines.push_back(line);
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
        auto var_type = JZNodeType::variantType(param.value);
        if (var_type == Type_string)
        {
            return "R\"(" + param.value.toString() + ")\"";
        }
        else if (var_type == Type_class)
        {
            return m_env.typeToName(var_type) + "()";
        }
        else
        {
            return JZNodeType::debugString(param.value);
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

        if (JZNodeType::isBase(ir_alloc->type))
        {
            line += " = " + m_env.defaultValueString(ir_alloc->dataType);
        }
        else if (JZNodeType::isPointer(ir_alloc->type))
        {
            line += " = nullptr";
        }
        line += ";";
        if (isFunctionInParam(ir_alloc->dst))
            line = "// " + line;
        break;
    }
    case OP_clearReg:
        line += "// clear reg";
        break;
    case OP_set:
    {
        JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
        line += toString(ir_set->dst) + " = " + toString(ir_set->src) + ";";
        if (isRegInParam(ir_set->dst)) //调用函数
        {
            line = "auto " + line;
        }
        if (isRegInParam(ir_set->src)) //函数入参
        {
            line = "// " + line;
        }
        if (isRegOutParam(ir_set->dst)) //函数出参
        {
            line = "// " + line;
        }
        break;
    }
    case OP_clone:
    {
        JZNodeIRClone *ir_set = (JZNodeIRClone*)op;
        line += toString(ir_set->dst) + " = " + toString(ir_set->src) + "; //clone";
        break;
    }
    case OP_buffer:
    {
        JZNodeIRBuffer *ir_set = (JZNodeIRBuffer*)op;
        line += toString(ir_set->id) + QString::asprintf("= QByteArray(%d);",ir_set->buffer.size());
        break;
    }
    case OP_convert:
    {
        JZNodeIRConvert *ir_cvt = (JZNodeIRConvert*)op;
        line += toString(ir_cvt->dst) + " = (" + m_env.typeToName(ir_cvt->dstType) + ")" + toString(ir_cvt->src) + ";";
        break;
    }
    case OP_call:
    {
        JZNodeIRCall *ir_call = (JZNodeIRCall *)op;
        line += dealCall(ir_call->function);

        if (isIrSetReg(op->pc + 1, SrcRegOut))
        {
            line = "auto Reg_CallOut_0 = " + line;
        }
        break;
    }
    case OP_return:
    {
        if (isIrSetReg(op->pc - 1, DstRegOut))
        {
            JZNodeIRSet* ir_set = dynamic_cast<JZNodeIRSet*>(m_scriptImpl->statmentList[op->pc - 1].data());
            line = "return " + toString(ir_set->src) + ";";
        }
        else
            line += "return;";
        break;
    }
    case OP_exit:
        line += "exit(0);";
        break;
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
    QString line;
    int reg_idx = 0;
    if (func_def->isMemberFunction())
    {
        line = JZNodeCompiler::paramName(Reg_CallIn + reg_idx++) + ".";
    }
    QStringList param_in_list;
    for(int i = reg_idx; i < func_def->paramIn.size(); i++)
        param_in_list << JZNodeCompiler::paramName(Reg_CallIn + i);
    line = line + func_def->name + "(" + param_in_list.join(",") + ")";
    if (func_def->paramOut.size() != 0)
        line = JZNodeCompiler::paramName(Reg_CallOut) + " = " + line + ";";

    return line;
}