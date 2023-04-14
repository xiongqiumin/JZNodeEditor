#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "FunctionParser.h"

class FuncGen
{
public:
    FuncGen();

    void addInclude(QString filepath);
    void generate();
    void dump(QString filepath, QString text);

    QList<FunctionDefine> funcList;

protected:
    QString pre();
    QString tab(int num);

    QMap<QString, QString> m_typeMap;
    int m_tab;
};

FuncGen::FuncGen()
{
    m_tab = 0;

    m_typeMap["int"] = "Type_int";
    m_typeMap["double"] = "Type_double";
}

QString FuncGen::pre()
{
    return tab(m_tab);
}

QString FuncGen::tab(int num)
{
    QString str;
    str.resize(num * 4, ' ');
    return str;
}

/*
 C const
 R refrence
 P point
 A array

 i int32
 d double

 */
void FuncGen::generate()
{
    QString body, init_func;
    QMap<QString, FunctionDefine> functypeMap;

    init_func += tab(1) + "JZNodeFunction *func = nullptr;\n";
    for (int i = 0; i < funcList.size(); i++)
    {
        auto func = funcList[i];
        QString enum_type = "Func_" + func.enumDeclare();
        if (!functypeMap.contains(enum_type))
            functypeMap.insert(enum_type, func);

        m_tab = 1;
        init_func += pre() + "func = new JZNodeFunction();\n";
        init_func += pre() + "func->setName(\"" + func.name + "\");\n";
        init_func += pre() + "func->functionName = \"" + func.name + "\";\n";
        init_func += pre() + "func->functionType = " + enum_type + ";\n";
        init_func += pre() + "func->functionPointer = (void*)" + func.name + ";\n";
        for (int i = 0; i < func.paramList.size(); i++)
        {
            auto &p = func.paramList[i];
            init_func += pre() + "func.addProp(JZNodePin(\"" + p.name + "\"," + m_typeMap[p.type] + ",Prop_in));\n";
        }
        if (func.hasReturn())
            init_func += pre() + "func.addProp(JZNodePin(\"result\"," + m_typeMap[func.result.type] + ",Prop_out));\n";
        init_func += pre() + "m_nodeFuncs.insert(func->functionName,JZNodePtr(func));\n";
        init_func += "\n";
    }

    body += "enum\n{\n";
    m_tab = 1;
    body += pre() + "Func_none,\n";
    auto it_e = functypeMap.begin();
    while (it_e != functypeMap.end())
    {
        body += pre() + it_e.key() + ",\n";
        it_e++;
    }
    m_tab = 0;
    body += "};\n\n";

    body += "void JZNodeFunctionManager::initBuildinFunction()\n{\n";
    body += init_func;
    body += "}\n";

    body += "void JZNodeFunctionManager::callFunction(int type,void *ptr,const QVariantList &paramIn,QVariantList &paramOut)\n{\n";
    m_tab = 1;
    body += pre() + "switch(type)\n" + pre() + "{\n";

    auto it = functypeMap.begin();
    while (it != functypeMap.end())
    {
        body += pre() + "case " + it.key() + ":\n" + pre() + "{\n";
        m_tab++;

        auto &func = it.value();
        bool isRet = (func.result.type != "void");

        QStringList params;
        for (int i = 0; i < func.paramList.size(); i++)
        {
            QString p = "p" + QString::number(i);
            QString def = QString("%1 p%2 = paramIn[%2].value<%1>();\n").arg(func.paramList[i].type, QString::number(i));
            body += pre() + def;
            params.append(p);
        }

        QString func_call;
        func_call = "((" + func.pointDeclare() + ")ptr)(" + params.join(",") + ");\n";
        if (isRet)
        {
            func_call = func.result.type + " ret = " + func_call;
            func_call += pre() + "paramOut.push_back(ret);\n";
        }

        body += pre() + func_call;
        body += pre() + "break;\n";
        m_tab--;
        body += pre() + "}\n";
        it++;
    }

    body += pre() + "default:\n" + tab(m_tab + 1) + "break;\n";

    body += pre() + "}\n";
    m_tab--;
    body += "}\n";
    dump("func_wrapper_gen.cpp", body);
}

void FuncGen::addInclude(QString filepath)
{
    FunctionParser parser;
    funcList.append(parser.parse(filepath));
}

void FuncGen::dump(QString filepath, QString text)
{
    QFile file(filepath);
    if (file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
        QTextStream s(&file);
        s << text;
        file.close();
    }
}

int jznode_add(int a, int b)
{
    return a + b;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FuncGen gen;
    gen.addInclude("C:/work/xiong/test/func.h");
    gen.generate();

    return 0;
}
