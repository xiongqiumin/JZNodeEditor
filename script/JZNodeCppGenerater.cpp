#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include "JZNodeCppGenerater.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBuilder.h"
#include "JZUiFile.h"
#include "JZContainer.h"

bool isValueType(int dataType)
{
    Q_ASSERT(dataType != Type_none);
    if(dataType <= Type_class)
        return true;

    auto meta = JZNodeObjectManager::instance()->meta(dataType);
    return meta->isValueType();
}

//CppFunction

//CppClass
CppClass::CppClass()
{
    qDeleteAll(functions);
}

CppClass::~CppClass()
{

}

CppFunction *CppClass::getFunction(QString func_name)
{
    for(int i = 0; i < functions.size(); i++)
    {
        if(functions[i]->define.fullName() == func_name)
            return functions[i];
    }

    auto func_def = JZNodeFunctionManager::instance()->function(func_name);

    CppFunction *func = new CppFunction();
    func->define = *func_def;
    functions.push_back(func);
    return functions.back();
}

//CppFile
CppFile::CppFile()
{
    qDeleteAll(classList);
    qDeleteAll(functionList);
}

CppFile::~CppFile()
{
}

CppClass *CppFile::getClass(QString name)
{
    for(int i = 0; i < classList.size(); i++)
    {
        if(classList[i]->name == name)
            return classList[i];
    }

    CppClass *cls = new CppClass();
    cls->name = name;
    classList.push_back(cls);
    return cls;
}

CppFunction *CppFile::getFunction(QString name)
{
    for(int i = 0; i < functionList.size(); i++)
    {
        if(functionList[i]->define.fullName() == name)
            return functionList[i];
    }

    auto func_def = JZNodeFunctionManager::instance()->function(name);
    
    CppFunction *func = new CppFunction();
    func->define = *func_def;
    functionList.push_back(func);
    return func;
}

//CppProgram
CppProgram::CppProgram()
{

}

CppProgram::~CppProgram()
{

}

CppFile *CppProgram::getFile(QString name)
{
    for(int i = 0; i < files.size(); i++)
    {
        if(files[i].path == name)
            return &files[i];
    }

    CppFile file;
    file.path = name;
    files.push_back(file);
    return &files.back();
}

//JZNodeCppGenerater
JZNodeCppGenerater::JZNodeCppGenerater()
{
    m_program = nullptr;
    m_function = nullptr;
    m_project = nullptr;
    m_tab = 0;

    m_macro = R"(#define JMP(pc) goto Line##pc
#define JE(pc)  do{ if(REG_CMP) goto Line##pc; }while(0)
#define JNE(pc) do{ if(!REG_CMP) goto Line##pc; }while(0))";
}
    
JZNodeCppGenerater::~JZNodeCppGenerater()
{
}

void JZNodeCppGenerater::saveFile(const QString &content,QString path)
{
    QFile file(path);
    if(!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
    {
        qDebug() << "save file failed\n";
        return;
    }

    QTextStream s(&file);
    s.setCodec("utf-8");
    s << content;
}

QString JZNodeCppGenerater::typeName(QString type)
{
    int id = JZNodeType::nameToType(type);
    return typeName(id);
}

QString JZNodeCppGenerater::typeName(int type)
{
    QString name = JZNodeType::typeToName(type);
    if(type == Type_nullptr)
    {
        return "void*";
    }
    else if(isValueType(type))
        return name;
    else
    {
        if(name.contains("<"))
        {
            auto tmp = parseTemplate(name);
            QStringList args;
            for(int i = 0; i < tmp.args.size(); i++)
                args << typeName(tmp.args[i]);

            name = tmp.name + "<" + args.join(",") + ">";
        }

        return "QSharedPointer<" + name + ">";
    }
}

QString JZNodeCppGenerater::idName(int id)
{   
    if(id == Reg_Cmp)
        return "REG_CMP";
    else if(id >= Reg_CallIn)
    {   
        if(id < Reg_CallOut)
            return "Reg_CallIn_" + QString::number(id - Reg_CallIn);
        else
            return "Reg_CallOut_" + QString::number(id - Reg_CallOut);
    }
    else if(id >= Reg_Start)
        return "Reg_" + QString::number(id - Reg_Start);
    else
        return "_Local_" + QString::number(id);
}

bool JZNodeCppGenerater::isRef(JZNodeIRParam param)
{
    if(param.isThis())
        return false;
    else if(param.isLiteral())
        return false;

    if(param.isStack())
    {
        Q_ASSERT_X(m_nodeType.contains(param.id()),"Id",qUtf8Printable(QString::number(param.id())));
        return m_nodeType[param.id()].isRef;
    }
    else
    {
        return false;
    }
}

int JZNodeCppGenerater::paramType(const JZNodeIRParam &param)
{
    if (param.type == JZNodeIRParam::Literal)
        return JZNodeType::variantType(param.value);
    else if(param.type == JZNodeIRParam::This)
        return m_file->getClassFile()->classType();
    else if(param.type == JZNodeIRParam::Reference)
    {
        auto def = JZNodeCompiler::getVariableInfo(m_file,param.ref());
        return def->dataType();
    }
    else
    {
        return m_nodeType[param.id()].dataType;
    }
}

QString JZNodeCppGenerater::paramToString(const JZNodeIRParam &param)
{
    if (param.type == JZNodeIRParam::Literal)
    {
        int v_type = JZNodeType::variantType(param.value);
        if (v_type == Type_string)
        {
            return "R\"(" + param.value.toString() + ")\"";
        }
        else if (v_type == Type_nullptr)
        {
            return "nullptr";
        }
        else if (v_type == Type_function)
        {
            auto func = param.value.value<JZFunctionPointer>();
            return "\"" + func.functionName + "\"";
        }
        else
            return param.literal().toString();
    }
    else if(param.type == JZNodeIRParam::Reference)
    {
        QString name = param.ref();
        name.replace("this.","thisPtr->");
        return name;
    }
    else if(param.type == JZNodeIRParam::This)
        return "thisPtr";
    else
        return idName(param.id());
}

QString JZNodeCppGenerater::tab()
{
    return QString().leftJustified(m_tab * 4); 
}

QString JZNodeCppGenerater::dealCall()
{
    auto func_def = JZNodeFunctionManager::instance()->function(m_irCall->function);
    QString func_name = func_def->name;

    QString line;
    QString reg_out;
    if(func_def->paramOut.size() > 0)
    {
        JZNodeIRParam id_reg = m_callOutList[0];
        reg_out = paramToString(id_reg) + " = ";
    }

    QString call;
    
    if(func_name == "createObject")
    {
        QString class_name = m_callInList[0].literal().toString();
        int class_type = JZNodeType::nameToType(class_name);
        if(isValueType(class_type))
            call = class_name + "()";
        else
        {
            QString type_name = typeName(class_name);
            type_name = type_name.mid(QString("QSharedPointer<").size());
            type_name.chop(1);

            call = "QSharedPointer<" + type_name + ">(new " + type_name + "()";
            if(JZNodeType::isInherits(class_name,"QObject"))
                call += ",QObjectDelete)";
            else
                call += ")";
        }
    }
    else
    {
        QStringList callInReg;
        for(int i = 0; i < m_irCall->inCount; i++)
        {
            QString name = paramToString(m_callInList[i]);
            if(func_def->isMemberFunction() && i == 0)
            {

            }
            else
            {
                bool is_dst_ref = !isValueType(func_def->paramIn[i].dataType());
                if(!func_def->isCFunction || func_def->fullName().contains("<"))
                    is_dst_ref = false;
                bool is_src_ref = isRef(m_callInList[i]);
                if(!is_dst_ref && is_src_ref)
                    name = "*" + name;
                else if(is_dst_ref && !is_src_ref)
                {
                    if(isValueType(func_def->paramIn[i].dataType()))
                        name = "&" + name;
                    else
                        name = name + ".data()";
                }
            }        
            callInReg << name;
        }
        
        if(func_def->isMemberFunction())
        {
            call += callInReg[0] + "->";
            callInReg.pop_front();
        }

        if(func_name == "connect")
        {   
            //auto signal = JZNodeObjectManager::instance()->signal();
            callInReg[1];
            callInReg[3];
        }

        if(func_def->className.contains("QList<"))
        {
            if(func_name == "get")
                func_name = "at";
            else if(func_name == "set")
                func_name = "replace";
        }

        if(func_name == "__fromString__" || func_name == "create")
            func_name = func_def->className + "_" + func_name;
        else if(!func_def->className.isEmpty() && !func_def->isMemberFunction())
            func_name = func_def->className + "::" + func_name;

        func_name.replace("<","_");
        func_name.replace(">","_");
        call += func_name + "(" + callInReg.join(",") + ")";

        if(func_def->className.contains("QList<") && func_def->name == "resize")
        {
            QString name = paramToString(m_callInList[0]);
            call = "List_resize(" + name + ".data(),"  + callInReg[0] + ")";
        }
    }

    line += tab() + reg_out + call + ";";

    m_irCall = nullptr;
    m_callInList.clear();
    m_callOutList.clear();
    
    return line; 
}

QString JZNodeCppGenerater::irToCpp(JZNodeIR *op)
{
    auto debug_info = debugInfo();

    QString line;
    switch (op->type)
    {
    case OP_nodeId:
        line += tab() + "//node";
        break;
    case OP_nop:
        line += tab() + "//nop";
        break;
    case OP_alloc:
    {
        JZNodeIRAlloc *ir_alloc = (JZNodeIRAlloc*)op;
        QString type = typeName(ir_alloc->dataType);

        line = tab();
        if (ir_alloc->allocType == JZNodeIRAlloc::Heap || ir_alloc->allocType == JZNodeIRAlloc::Stack)
        {
            bool is_func_input = false;
            if(ir_alloc->allocType == JZNodeIRAlloc::Stack)
            {
                for(int i = 0; i < m_function->define.paramIn.size(); i++)
                {
                    if(m_function->define.paramIn[i].name == ir_alloc->name)
                    {
                        is_func_input = true;
                        break;
                    }
                }
            }

            if(!is_func_input)
                line += type + " " + ir_alloc->name;
            else
                line += "//" + ir_alloc->name;
        }
        else
        {   
            NodeInfo info;
            info.dataType = ir_alloc->dataType;
            if(info.dataType < Type_class)
                info.isRef = false;
            else
            {
                if(isValueType(info.dataType))
                {
                    info.isRef = true;
                    if(ir_alloc->id < Stack_User)
                    {
                        auto gemo = JZNodeCompiler::paramGemo(ir_alloc->id);
                        auto node = m_file->getNode(gemo.nodeId);
                        if((node->type() == Node_function || node->type() == Node_create
                            || node->type() == Node_createFromString) && node->paramOutList().contains(gemo.pinId))
                        {
                            info.isRef = false;
                        }
                    }
                }
                else
                {
                    info.isRef = false;
                }
            }
            m_nodeType[ir_alloc->id] = info;
            if(info.isRef)
                type += "*";

            line += type + " " + idName(ir_alloc->id);
        }

        line += ";";
        break;
    }
    case OP_clearReg:
    {
        line += tab() + "//clearReg";
        break;
    }
    case OP_set:
    {   
        JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
        if(ir_set->src.isReg() && ir_set->src.id() < Reg_CallOut) //函数调用，寄存器到入参
        {
            line = tab() + "//regIn -> local";   
        }
        else if(ir_set->dst.isReg() && ir_set->dst.id() < Reg_CallOut)
        {
            line = tab() + "//local -> regIn";
            m_callInList.push_back(ir_set->src);
        }
        else if(ir_set->src.isReg() && ir_set->src.id() >= Reg_CallOut)
        {
            m_callOutList.push_back(ir_set->dst);
            line = dealCall();
        }
        else if(ir_set->dst.isReg() && ir_set->dst.id() >= Reg_CallOut)
        {
            line = tab() + "//local -> regOut";
            m_returnList.push_back(ir_set->src);
        }
        else
        {
            QString src_name,dst_name;
            src_name = paramToString(ir_set->src);
            dst_name = paramToString(ir_set->dst);
            bool s_ref = isRef(ir_set->src);
            bool d_ref = ir_set->dst.isReg()? s_ref : isRef(ir_set->dst);

            if(JZNodeType::isEnum(paramType(ir_set->dst)) && paramType(ir_set->src) == Type_int)
            {
                QString enum_name = JZNodeType::typeToName(paramType(ir_set->dst));
                src_name = "(" + enum_name + ")" + src_name;
            }

            QString assgin;
            if((s_ref && d_ref) || (!s_ref && !d_ref))
                assgin = dst_name + " = " + src_name;
            else if(s_ref && !d_ref)
                assgin = dst_name + " = *" + src_name;
            else
                assgin = dst_name + " = &" + src_name;

            line += tab() + assgin + ";";
        }
        break;
    }
    case OP_clone:
    {
        JZNodeIRClone *ir_set = (JZNodeIRClone*)op;
        if(ir_set->src.isReg() && ir_set->src.id() < Reg_CallOut) 
        {
            Q_ASSERT(0);   
        }
        else if(ir_set->dst.isReg() && ir_set->dst.id() < Reg_CallOut)
        {
            line = tab() + "//local -> regIn";
            m_callInList.push_back(ir_set->src);
        }
        else if(ir_set->src.isReg() && ir_set->src.id() >= Reg_CallOut)
        {
            m_callOutList.push_back(ir_set->dst);
            line = dealCall();
        }
        else if(ir_set->dst.isReg() && ir_set->dst.id() >= Reg_CallOut)
        {
            line = tab() + "//local -> regOut";
            m_returnList.push_back(ir_set->src);
        }
        else
        {
            QString src_name,dst_name;
            src_name = paramToString(ir_set->src);
            dst_name = paramToString(ir_set->dst);
            bool s_ref = isRef(ir_set->src);
            if(s_ref)
                src_name = "*" + src_name;
            line += tab() + " " + dst_name + " = " + src_name + ";";
        }
        break;
    }
    case OP_convert:
    {
        JZNodeIRConvert *ir_cvt = (JZNodeIRConvert*)op;
        QString src_name,dst_name;
        src_name = paramToString(ir_cvt->src);
        dst_name = paramToString(ir_cvt->dst);
        QString type_name = typeName(ir_cvt->dstType);

        if(paramType(ir_cvt->dst) != Type_nullptr && paramType(ir_cvt->src) == Type_nullptr)
            line += tab() + type_name + "();";
        else
            line += tab() + dst_name + " = (" + type_name + ")" + src_name + ";";
        break;
    }
    case OP_watch:
    {
        line += tab() + "//watch";
        break;
    }
    case OP_call:
    {
        JZNodeIRCall *ir_call = (JZNodeIRCall *)op;
        m_irCall = ir_call;

        auto func_def = JZNodeFunctionManager::instance()->function(ir_call->function);
        if(func_def->paramOut.size() == 0)
            line = dealCall();
        break;
    }
    case OP_return:
    {
        if(m_returnList.size() == 0)
            line += tab() + "return;";
        else
            line += tab() + "return " + paramToString(m_returnList[0]) + ";";
        m_returnList.clear();
        break;
    }
    case OP_exit:
        line += tab() + "exit(0);";
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
        QString c = paramToString(ir_expr->dst);
        QString a = paramToString(ir_expr->src1);
        QString b = paramToString(ir_expr->src2);
        line += tab() + c + " = " + a + " " + JZNodeType::opName(op->type) + " " + b + ";";
        break;
    }
    case OP_not:
    {
        JZNodeIRExpr *ir_expr = (JZNodeIRExpr *)op;
        QString c = paramToString(ir_expr->dst);
        QString a = paramToString(ir_expr->src1);
        line += tab() + c + " =  !" + a + ";";
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

        line = tab() + line;
        if(!m_jumpAddr.contains(ir_jmp->jmpPc))
            m_jumpAddr.push_back(ir_jmp->jmpPc);
        break;
    }
    case OP_assert:
    {
        break;
    }
    default:
        Q_ASSERT(0);
        break;
    }

    return line;
}

JZFunctionDebugInfo *JZNodeCppGenerater::debugInfo()
{
    return m_script->functionDebug(m_function->fullName());
}

bool JZNodeCppGenerater::toCppFunction(JZFunction *func,CppFunction *cfunc)
{
    m_file = m_project->functionItem(func->fullName());
    m_function = func;
    cfunc->define = func->define;
    m_tab = 1;

    m_irCall = nullptr;
    m_jumpAddr.clear();
    m_nodeType.clear();
    m_callInList.clear();
    m_callOutList.clear();
    m_returnList.clear();

    QStringList lines;
    auto script = m_program->script(func->path);
    for(int i = func->addr; i < func->addrEnd; i++)
    {
        auto stmt = script->statmentList[i].data();
        lines << irToCpp(stmt);
    }
    std::sort(m_jumpAddr.begin(), m_jumpAddr.end());
    for(int i = m_jumpAddr.size() - 1; i >= 0; i--)
    {
        int addr = m_jumpAddr[i];
        lines.insert(addr - func->addr,"Line" + QString::number(addr) + ":");
    }
    cfunc->code = lines.join("\n");
    
    return true;
}

void JZNodeCppGenerater::generate(JZProject *project,QString output)
{
    m_project = project;

    JZNodeBuilder builder;
    builder.setProject(m_project);
    
    JZNodeProgram program;
    if(!builder.build(&program))
    {
        qDebug() << "build program failed";
        return;
    }
    m_program = &program; 

    QSet<CppClass*> class_set;

    auto script_list = m_program->scriptList();
    for(int script_idx = 0; script_idx < script_list.size(); script_idx++)
    {
        auto script = script_list[script_idx];
        if(script->file == "__init__")
            continue;
        
        m_script = script;
        auto script_item = m_project->getItem(script->file);
        QString file_path = m_project->getItemFile(script_item)->itemPath();
    
        CppFile *cpp_file = m_cppProgram.getFile(file_path); 
        CppClass *cpp_class = nullptr;
        if(!script->className.isEmpty())
        {
            cpp_class = cpp_file->getClass(script->className);
            if(!class_set.contains(cpp_class))
            {
                class_set.insert(cpp_class);

                auto jz_class = m_project->getClass(cpp_class->name);
                cpp_class->super = jz_class->superClass();
                QStringList params = jz_class->memberVariableList(false);
                for(int i = 0; i < params.size(); i++)
                {
                    auto param = jz_class->memberVariable(params[i],false);
                    cpp_class->params.push_back(*param);
                }

                QString uiFile = jz_class->uiFile();
                if(!uiFile.isEmpty())
                {
                    auto ui_item = (JZUiFile*)m_project->getItem(uiFile);
                    QString ui_xml = ui_item->xml();
                    int cls_idx = ui_xml.indexOf("<class>");
                    cls_idx += 7;
                    int cls_idx_end = ui_xml.indexOf("</class>",cls_idx);
                    QString ui_class = ui_xml.mid(cls_idx,cls_idx_end - cls_idx);
                    
                    if(uiFile.startsWith("./"))
                        uiFile = uiFile.mid(2);

                    cpp_class->uiClass = ui_class;
                    cpp_class->uiFile = uiFile;
                    
                    CppUiFile cpp_ui;
                    cpp_ui.path = uiFile;
                    cpp_ui.xml = ui_xml;
                    m_cppProgram.uis.push_back(cpp_ui);
                }
            }   
        }

        auto funcs = script->functionList;
        for(int func_idx = 0; func_idx < funcs.size(); func_idx++)
        {
            auto func = &funcs[func_idx];
            CppFunction *cpp_func = nullptr;
            if(cpp_class)
                cpp_func = cpp_class->getFunction(func->fullName());
            else
                cpp_func = cpp_file->getFunction(func->fullName());
            
            toCppFunction(func,cpp_func);
        }
    }

    dumpProgram(output);
}

QString JZNodeCppGenerater::functionDeclare(CppFunction *func)
{
    auto &define = func->define;

    QString returnType = "void";
    if(define.paramOut.size() > 0)
        returnType = typeName(define.paramOut[0].type);

    QString line = returnType + " " + define.name + "(";
    QStringList param_str;
    int start = define.isMemberFunction()? 1:0;
    for(int i = start; i < define.paramIn.size(); i++)
    {
        auto p = define.paramIn[i];
        param_str.push_back(typeName(p.dataType()) + " " + p.name);
    }
    return line + param_str.join(",") + ")";
}

QString JZNodeCppGenerater::functionImpl(CppFunction *func)
{
    auto &define = func->define;

    QString content = functionDeclare(func) + "\n{\n";
    if(!define.className.isEmpty())
    {
        int idx = content.indexOf(" ");
        content.insert(idx+1, define.className + "::");
    }
    content += func->code + "\n}";
    return content;
}

QString JZNodeCppGenerater::classDeclare(CppClass *cls)
{
    QString name = cls->name;
    QString super = cls->super;
    QString uiClass = cls->uiClass;
    QString uiFile = cls->uiFile;
    QList<CppFunction*> functions = cls->functions;
    QList<JZParamDefine> params = cls->params;
    int class_type = JZNodeType::nameToType(name);

    QString space = "    ";
    QString code;
    
    if(!uiClass.isEmpty())
        code += "namespace Ui { class " + uiClass + "; }\n\n";

    code += "class " + name;
    if(!super.isEmpty())
        code += " : public " + super; 
    code += "\n{\n";

    code += "public:\n";
    code += space + name + "();\n";
    code += space + "virtual ~" + name + "();\n\n";

    for(int i = 0; i < functions.size(); i++)
    {
        code += space + functionDeclare(functions[i]) + ";\n";
    }
    if(params.size() > 0)
        code += "\n";
    for(int i = 0; i < params.size(); i++)
    {
        code += space + typeName(params[i].dataType()) + " " + params[i].name + ";\n";
    }
    code += space + typeName(class_type) + " thisPtr;";

    if(!uiClass.isEmpty())
        code += "\n" + space + "Ui::" + uiClass + " *ui;\n";

    code += "};";
    return code;
}

QString JZNodeCppGenerater::classImpl(CppClass *cls)
{
    QString name = cls->name;
    QString super = cls->super;
    QString uiClass = cls->uiClass;
    QString uiFile = cls->uiFile;
    QList<CppFunction*> functions = cls->functions;
    QList<JZParamDefine> params = cls->params;
    int class_type = JZNodeType::nameToType(name);

    QString code;
    QString space = "    ";

    //co
    code += name + "::" + name + "()\n{\n";
    if(!uiClass.isEmpty())
    {
        code += space + "ui = new Ui::" + uiClass + "();\n";
        code += space + "ui->setupUi(this);\n";
        if(params.size() > 0)
            code += "\n";    
    }
    for(int i = 0; i < params.size(); i++)
    {
        auto p = params[i];
        if(p.dataType() <= Type_double)
        {
            code += space + p.name + " = ";
            if(!p.value.isEmpty())
                code += p.value;
            else 
            {
                if(p.dataType() == Type_bool)
                    code += "true";
                else if(p.dataType() <= Type_double)
                    code += "0";
            }
            code +=  ";\n";
        }
    }
    code += space + "thisPtr = " + typeName(class_type) + "(this,EmptyDelete<" + name + ">);\n";
    code += "}\n\n";

    //deco
    code += name + "::~" + name + "()\n{\n";
    code += "}\n\n";
    
    for(int i = 0; i < functions.size(); i++)
    {
        code += functionImpl(functions[i]) + "\n\n";
    }
    return code;
}

bool JZNodeCppGenerater::dumpProgram(QString output)
{
    QDir dir;
    if(!dir.exists(output))
        dir.mkdir(output);

    QStringList lib_list;
    QStringList include_list;
    QStringList h_list;
    QStringList cpp_list;
    QStringList ui_list;

    for(int file_idx = 0; file_idx < m_cppProgram.files.size(); file_idx++)
    {
        auto &file = m_cppProgram.files[file_idx];
        auto &func_list = file.functionList;
        auto &cls_list = file.classList;

        QFileInfo info(file.path);
        QString file_name = info.baseName();
        QString file_path = info.path() + "/" + file_name;
        if(file_path.startsWith("./"))
            file_path = file_path.mid(2);

        QString out_h_path = output + "/" + file_path + ".h";
        QString out_cpp_path = output + "/" + file_path + ".cpp";
        h_list << file_path + ".h";
        cpp_list << file_path + ".cpp";

        QString out_h;
        QString out_cpp;

        out_cpp += "#include \"" + file_name + ".h\"\n";
        
        for(int cls_idx = 0; cls_idx < cls_list.size(); cls_idx++)
        {
            auto ui_file = cls_list[cls_idx]->uiFile;
            if(!ui_file.isEmpty())
            {
                int idx = ui_file.lastIndexOf("/");
                ui_file.insert(idx+1,"ui_");
            }
            ui_file.chop(3);
            out_cpp += "#include \"" + ui_file + ".h\"\n";
        }

        out_cpp += "\n";
        out_cpp += m_macro + "\n\n";

        if(file_path == "main")
        {
            auto global_list = m_project->globalVariableList();
            for(int i = 0; i < global_list.size(); i++)
            {
                auto param = m_project->globalVariable(global_list[i]);
                //out_cpp += param->type + " " + param->name + " = " + param->initValue() + ";\n";
            }

            out_cpp += "\n";
        }

        out_h += "#ifndef " + file_name.toUpper() + "_H_\n";
        out_h += "#define " + file_name.toUpper() + "_H_\n\n";
        out_h += "#include \"JZRuntime.h\"\n\n";

        for(int func_idx = 0; func_idx < func_list.size(); func_idx++)
        {
            auto func = func_list[func_idx];
            out_h += functionDeclare(func) + ";\n";
            out_cpp += functionImpl(func) + "\n\n";

            if(func->define.fullName() == "main")
            {
                QString space = "    ";
                out_cpp += "int main(int argc, char *argv[])\n{\n";
                out_cpp += space + "QApplication app(argc, argv);\n\n";
                out_cpp += space + "main();\n\n";
                out_cpp += space + "return app.exec();\n}\n\n";
            }
        }

        if(func_list.size() > 0 && cls_list.size() > 0)
        {
            out_cpp += "\n";
            out_h += "\n";
        }

        for(int cls_idx = 0; cls_idx < cls_list.size(); cls_idx++)
        {
            auto cls = cls_list[cls_idx];
            out_h += classDeclare(cls) + "\n";
            out_cpp += classImpl(cls) + "\n\n";
        }

        out_h += "\n#endif\n";

        saveFile(out_cpp,out_cpp_path); 
        saveFile(out_h,out_h_path);
    }

    for(int file_idx = 0; file_idx < m_cppProgram.uis.size(); file_idx++)
    {
        auto &ui = m_cppProgram.uis[file_idx];
        QString out_ui_path = output + "/" + ui.path;
        saveFile(ui.xml,out_ui_path);

        qDebug() << "ui" << out_ui_path << ui.path;
        ui_list << ui.path;
    }

    //pro
    QString out_pro;
    out_pro += "QT += core gui widgets\n\n";

    auto proAddList = [&out_pro](QString title,QStringList list){
        if(list.size() == 0)
            return;

        out_pro += title + " += \\\n";
        for(int i = 0; i < list.size(); i++)
        {
            out_pro += "    " + list[i];
            if(i != list.size() - 1)
                out_pro += " \\\n";
        }
        out_pro += "\n\n";
    };

    h_list << "JZRuntime.h";
    cpp_list << "JZRuntime.cpp";
    
    proAddList("LIBS",lib_list);
    proAddList("INCLUDEPATH",include_list);
    proAddList("HEADERS",h_list);
    proAddList("SOURCES",cpp_list);
    proAddList("FORMS",ui_list);

    saveFile(out_pro,output + "/" + m_project->name() + ".pro");
    return true;
}