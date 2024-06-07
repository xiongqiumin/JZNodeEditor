#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include "JZNodeGenerateCpp.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBuilder.h"
#include "JZUiFile.h"

constexpr int Reg_Call_In = Reg_Call;
constexpr int Reg_Call_Out = Reg_Call + 20;

//CppFunction
QString CppFunction::declare()
{
    QString line = returnType + " " + name + "(";
    QStringList param_str;
    int start = className.isEmpty()? 0:1;
    for(int i = start; i < params.size(); i++)
        param_str.push_back(params[i].type + " " + params[i].name);

    return line + param_str.join(",") + ")";
}

QString CppFunction::impl()
{
    QString content = declare() + "\n{\n";
    if(!className.isEmpty())
    {
        int idx = content.indexOf(" ");
        content.insert(idx+1,className + "::");
    }
    content += code + "\n}";
    return content;
}

//CppClass
CppClass::CppClass()
{
    qDeleteAll(functions);
}

CppClass::~CppClass()
{

}

CppFunction *CppClass::getFunction(QString name)
{
    for(int i = 0; i < functions.size(); i++)
    {
        if(functions[i]->name == name)
            return functions[i];
    }

    CppFunction *func = new CppFunction();
    func->name = name;
    functions.push_back(func);
    return functions.back();
}

QString CppClass::declare()
{
    QString space = "    ";
    QString code;
    
    if(!uiClass.isEmpty())
        code += "namespace Ui { class " + uiClass + "; }\n\n";

    code += "class " + name;
    if(!super.isEmpty())
        code += " public: " + super; 
    code += "\n{\n";

    code += "public:\n";
    code += space + name + "();\n";
    code += space + "virtual ~" + name + "();\n\n";

    for(int i = 0; i < functions.size(); i++)
    {
        code += space + functions[i]->declare() + ";\n";
    }
    if(params.size() > 0)
        code += "\n";
    for(int i = 0; i < params.size(); i++)
    {
        code += space + params[i].type + " " + params[i].name + ";\n";
    }

    if(!uiClass.isEmpty())
        code += "\n" + space + "Ui::" + uiClass + " *ui;\n";

    code += "};";
    return code;
}

QString CppClass::impl()
{
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
        code += space + params[i].name + " = " + params[i].value + ";";
    }
    code += "}\n\n";

    //deco
    code += name + "::~" + name + "()\n{\n";
    code += "}\n\n";
    
    for(int i = 0; i < functions.size(); i++)
    {
        code += functions[i]->impl() + "\n\n";
    }
    return code;
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
        if(functionList[i]->name == name)
            return functionList[i];
    }

    CppFunction *func = new CppFunction();
    func->name = name;
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

//JZNodeGenerateCpp
JZNodeGenerateCpp::JZNodeGenerateCpp()
{
    m_program = nullptr;
    m_function = nullptr;
    m_project = nullptr;
    m_tab = 0;

    m_macro = R"(#define JMP(pc) goto Line##pc
#define JE(pc)  do{ if(REG_CMP) goto Line##pc; }while(0)
#define JNE(pc) do{ if(!REG_CMP) goto Line##pc; }while(0))";
}
    
JZNodeGenerateCpp::~JZNodeGenerateCpp()
{
}

void JZNodeGenerateCpp::saveFile(const QString &content,QString path)
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

QString JZNodeGenerateCpp::idName(int id)
{   
    if(id == Reg_Cmp)
        return "Reg_Cmp";
    else if(id >= Reg_Call)
    {   
        if(id < Reg_Call_Out)
            return "Reg_CallIn_" + QString::number(id - Reg_Call);
        else
            return "Reg_CallOut_" + QString::number(id - Reg_Call_Out);
    }
    else if(id >= Reg_Start)
        return "Reg_" + QString::number(id - Reg_Start);
    else
        return "_Local_" + QString::number(id);
}

QString JZNodeGenerateCpp::paramToString(JZNodeIRParam param)
{
    if (param.type == JZNodeIRParam::Literal)
    {
        if (param.value.type() == QVariant::String)
            return "\"" + param.value.toString() + "\"";
        else if (JZNodeType::isNullptr(param.value))
        {
            auto v = toJZNullptr(param.value);
            return "nullptr";
        }
        else
            return JZNodeType::toString(param.value);
    }
    else if(param.type == JZNodeIRParam::Reference)
    {
        QString name = param.ref();
        name.replace("this.","this->");
        return name;
    }
    else if(param.type == JZNodeIRParam::This)
        return "this";
    else
        return idName(param.id());
}

QString JZNodeGenerateCpp::irToCpp(JZNodeIR *op)
{
    QString space = QString().leftJustified(m_tab * 4);    

    QString line;
    switch (op->type)
    {
    case OP_nodeId:
        line += "//node";
        break;
    case OP_nop:
        line += "//nop";
        break;
    case OP_alloc:
    {
        JZNodeIRAlloc *ir_alloc = (JZNodeIRAlloc*)op;
        QString type = JZNodeType::typeToName(ir_alloc->dataType);
        if(JZNodeType::isObject(ir_alloc->dataType))
            type += "*";

        if (ir_alloc->allocType == JZNodeIRAlloc::Heap || ir_alloc->allocType == JZNodeIRAlloc::Stack)
        {
            bool is_func_input = false;
            if(ir_alloc->allocType == JZNodeIRAlloc::Stack)
            {
                for(int i = 0; i < m_function->paramIn.size(); i++)
                {
                    if(m_function->paramIn[i].name == ir_alloc->name)
                    {
                        is_func_input = true;
                        break;
                    }
                }
            }

            if(!is_func_input)
                line = type + " " + ir_alloc->name + " = " + paramToString(ir_alloc->value);
            else
                line = "//" + ir_alloc->name;
        }
        else
            line += type + " " + idName(ir_alloc->id) + " = " + paramToString(ir_alloc->value);

        line += ";";
        break;
    }
    case OP_set:
    {   
        bool no_commt = false;
        JZNodeIRSet *ir_set = (JZNodeIRSet*)op;
        QString src_name,dst_name;
        if(ir_set->dst.isId() && ir_set->dst.id() >= Reg_Call)
        {
            if(m_callInReg.size() == 0)
                m_tab++;

            dst_name = paramToString(ir_set->dst);
            m_callInReg.push_back(dst_name);
            dst_name = "auto " + dst_name;

            if(m_callInReg.size() == 1)
                dst_name = "{\n    " + space + dst_name;
        }
        else
        {
            dst_name = paramToString(ir_set->dst);
        }

        if(ir_set->src.isId() && ir_set->src.id() >= Reg_Call)
        {
            int reg_idx = ir_set->src.id() - Reg_Call;
            src_name = paramToString(irId(Reg_Call_Out + reg_idx));
            if(reg_idx == m_callOutReg.size() - 1)
            {
                m_callOutReg.clear();
                m_tab--;
                src_name += ";\n    }";
                no_commt = true;
            }
        }
        else
        {
            src_name = paramToString(ir_set->src);
        }

        line += dst_name + " = " + src_name;
        if(!no_commt)
            line += ";";
        break;
    }
    case OP_call:
    {
        JZNodeIRCall *ir_call = (JZNodeIRCall *)op;
        auto func_def = JZNodeFunctionManager::instance()->function(ir_call->function);
        if(func_def->paramOut.size() > 0)
        {
            Q_ASSERT(func_def->paramOut.size() == 1);

            QString ret_reg = paramToString(irId(Reg_Call_Out));
            line += "auto " + ret_reg + " = ";
            m_callOutReg.push_back(ret_reg);
        }

        if(!func_def->className.isEmpty())
        {
            line += m_callInReg[0] + "->";
            m_callInReg.pop_front();
        }
        line += func_def->name + "(" + m_callInReg.join(",") + ");";
        m_callInReg.clear();

        if(func_def->paramOut.size() == 0)
        {
            m_tab--;
            line += "\n    }";
        }
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
        QString c = paramToString(ir_expr->dst);
        QString a = paramToString(ir_expr->src1);
        QString b = paramToString(ir_expr->src2);
        line += c + " = " + a + " " + JZNodeType::opName(op->type) + " " + b + ";";
        break;
    }
    case OP_not:
    {
        JZNodeIRExpr *ir_expr = (JZNodeIRExpr *)op;
        QString c = paramToString(ir_expr->dst);
        QString a = paramToString(ir_expr->src1);
        line += c + " =  !" + a + ";";
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

    line = space + line;
    return line;
}

bool JZNodeGenerateCpp::toCppFunction(JZFunction *func,CppFunction *cfunc)
{
    m_function = func;
    cfunc->name = func->name;
    cfunc->className = func->className;
    m_tab = 1;
    
    for(int i = 0; i < func->paramIn.size(); i++)
    {   
        CppParam param;
        param.name = func->paramIn[i].name;
        param.type = JZNodeType::typeToName(func->paramIn[i].dataType);
        cfunc->params.push_back(param);
    }

    auto &paramOut = func->paramOut;
    if(paramOut.size() == 1)
        cfunc->returnType = JZNodeType::typeToName(paramOut[0].dataType);
    else
        cfunc->returnType = "void";

    m_jumpAddr.clear();
    QStringList lines;
    auto script = m_program->script(func->path);
    for(int i = func->addr; i < func->addrEnd; i++)
    {
        auto stmt = script->statmentList[i].data();
        lines << irToCpp(stmt);
    }
    for(int i = m_jumpAddr.size() - 1; i >= 0; i--)
    {
        int addr = m_jumpAddr[i];
        lines.insert(addr - func->addr,"Line" + QString::number(addr) + ":");
    }

    cfunc->code = lines.join("\n");
    
    return true;
}

void JZNodeGenerateCpp::generate(JZProject *project,QString output)
{
    m_project = project;

    JZNodeBuilder builder(m_project);
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
        
        QString file_path;
        if(script->path == "__init__")
        {
            continue;
        }
        else
        {
            auto script_item = m_project->getItem(script->path);
            file_path = m_project->getItemFile(script_item)->itemPath();
        }

        qDebug() << script->path << file_path << script->className;

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

                    CppParam cpp_param;
                    cpp_param.name = param->name;
                    cpp_param.type = param->type;
                    cpp_param.value = param->initValue(); 
                    cpp_class->params.push_back(cpp_param);
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
                cpp_func = cpp_class->getFunction(func->name);
            else
                cpp_func = cpp_file->getFunction(func->name);
            
            toCppFunction(func,cpp_func);
        }
    }

    dumpProgram(output);
}

bool JZNodeGenerateCpp::dumpProgram(QString output)
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
                out_cpp += param->type + " " + param->name + " = " + param->initValue() + ";\n";
            }

            out_cpp += "\n";
        }

        out_h += "#ifndef " + file_name.toUpper() + "_H_\n";
        out_h += "#define " + file_name.toUpper() + "_H_\n\n";

        for(int func_idx = 0; func_idx < func_list.size(); func_idx++)
        {
            auto func = func_list[func_idx];
            out_h += func->declare() + ";\n";
            out_cpp += func->impl() + "\n\n";

            if(func->name == "__main__")
            {
                QString space = "    ";
                out_cpp += "int main(int argc, char *argv[])\n{\n";
                out_cpp += space + "QApplication app(argc, argv);\n\n";
                out_cpp += space + "__main__();\n\n";
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
            out_h += cls->declare() + "\n";
            out_cpp += cls->impl() + "\n\n";
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
    
    proAddList("LIBS",lib_list);
    proAddList("INCLUDEPATH",include_list);
    proAddList("HEADERS",h_list);
    proAddList("SOURCES",cpp_list);
    proAddList("FORMS",ui_list);

    saveFile(out_pro,output + "/" + m_project->name() + ".pro");
    return true;
}