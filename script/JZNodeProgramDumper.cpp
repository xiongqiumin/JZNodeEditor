#include "JZNodeProgramDumper.h"
#include "JZNodeCompiler.h"    

JZNodeProgramDumper::JZNodeProgramDumper()
{    
    m_program = nullptr;
}

QString JZNodeProgramDumper::dump(JZNodeProgram *program)
{        
    m_program = program;
    m_program->initEnv(&m_env);

    QString content;    
    auto sc_list = m_program->scriptList();
    for(int sc_idx = 0; sc_idx < sc_list.size(); sc_idx++)
    {
        JZNodeScript *script = sc_list[sc_idx];
        content += "// Script " + script->file + "\n";
        auto &opList = script->statmentList;
        for(int func_idx = 0; func_idx < script->functionList.size(); func_idx++)
        {
            auto &func = script->functionList[func_idx];
            
            content += functionDeclare(&func) + "\n{\n";
            for(int i = func.addr; i < func.addrEnd; i++)
            {
                //deal op            
                content += irToString(opList[i].data()) + "\n";
            }
            content += "}\n\n";
        }
    }    
    return content;
}

QString JZNodeProgramDumper::toString(JZNodeIRParam param)
{
    if (param.type == JZNodeIRParam::Literal)
    {
        auto var_type = JZNodeType::variantType(param.value);
        if (var_type == Type_string)
            return "\"" + param.value.toString() + "\"";
        else if (var_type == Type_class)
        {
            return m_env.typeToName(var_type) + "()";
        }
        else 
            return JZNodeType::debugString(param.value) + ":" + m_env.typeToName(var_type);
    }
    else if(param.type == JZNodeIRParam::Reference)
        return param.ref();
    else if(param.type == JZNodeIRParam::This)
        return "this";
    else
        return JZNodeCompiler::paramName(param.id());
}

QString JZNodeProgramDumper::irToString(JZNodeIR *op)
{    
    QString line;
    switch (op->type)
    {
    case OP_nodeId:
    {
        JZNodeIRNodeId *ir_node = (JZNodeIRNodeId*)op;
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
            line += alloc + " " + ir_alloc->name + ";";
        else
            line += alloc + " " + JZNodeCompiler::paramName(ir_alloc->id) + ";";
        break;
    }
    case OP_clearReg:
        line += "//clear reg";
        break;
    case OP_set:
    {
        JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
        line += toString(ir_set->dst) + " = " + toString(ir_set->src) + ";";
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
    case OP_watch:
    {
        JZNodeIRWatch *ir_watch = (JZNodeIRWatch*)op;
        line += "//watch " + toString(ir_watch->traget) + " = " + toString(ir_watch->source);
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
        break;
    }
    case OP_return:
        line += "return;";
        break;
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
        line += c + " = " + a + " " + JZNodeType::opName(op->type) + " " + b;
        break;
    }
    case OP_not:
    {
        JZNodeIRExpr *ir_expr = (JZNodeIRExpr *)op;
        QString c = toString(ir_expr->dst);
        QString a = toString(ir_expr->src1);
        line += c + " = " + JZNodeType::opName(op->type) + a;
        break;
    }
    case OP_jmp:
    case OP_jne:
    case OP_je:
    {
        JZNodeIRJmp *ir_jmp = (JZNodeIRJmp *)op;
        if (op->type == OP_jmp)
            line += "JMP " + QString::number(ir_jmp->jmpPc);
        else if (op->type == OP_je)
            line += "JE " + QString::number(ir_jmp->jmpPc);
        else
            line += "JNE " + QString::number(ir_jmp->jmpPc);
        break;
    }
    case OP_assert:
    {
        JZNodeIRAssert *ir_assert = (JZNodeIRAssert *)op;
        line += "assert(" + toString(ir_assert->tips) + ")";
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
    line = QString::asprintf("%04d", op->pc) + "    " + line;
    return line;
}    

QString JZNodeProgramDumper::functionDeclare(JZFunction* func)
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