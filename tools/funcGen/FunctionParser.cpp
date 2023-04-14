#include "FunctionParser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

static QMap<QString,QString> typeMap = {
    {"void","v"},
    {"int","i"},
    {"double","d"},
};

void ParamDefine::parse(QString text)
{
    auto index = text.lastIndexOf(" ");
    type = text.mid(0,index);
    name = text.mid(index+1);
}

QString ParamDefine::defineType()
{
    return typeMap[type];
}

//FunctionDefine
void FunctionDefine::parse(QString content)
{
    int lbkt = content.indexOf("(");
    int rbkt = content.indexOf(")",lbkt);

    int index = content.lastIndexOf(" ",lbkt);
    QString ret_param = content.mid(0,index);
    result.parse(ret_param);
    result.name = "result";

    name = content.mid(index + 1,(lbkt - 1) - (index + 1) + 1);
    QStringList param_list = content.mid(lbkt + 1,(rbkt - 1) - (lbkt + 1) + 1).split(",");
    for(int i = 0; i < param_list.size(); i++)
    {
        ParamDefine param;
        param.parse(param_list[i]);
        paramList.push_back(param);
    }
}

bool FunctionDefine::hasReturn()
{
    return this->result.type != "void";
}

QString FunctionDefine::enumDeclare()
{
    QString line;
    for(int i = 0 ; i < paramList.size(); i++){
        line += paramList[i].defineType();
    }
    line += "_" + result.defineType();
    return line;
}

QString FunctionDefine::pointDeclare()
{
    QString text;
    QStringList params;
    for(int i = 0 ; i < paramList.size(); i++){
        params.push_back(paramList[i].type);
    }

    text += result.type + "(*)(" + params.join(",") + ")";

    return text;
}

//FunctionParser
FunctionParser::FunctionParser()
{

}

FunctionParser::~FunctionParser()
{

}

QList<FunctionDefine> FunctionParser::parse(QString filepath)
{
    QList<FunctionDefine> results;
    QString content;
    QFile file(filepath);
    if(file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream s(&file);
        content = s.readAll();        
    }

    QStringList func_list;

    int start = 0;
    while(start <= content.size())
    {
        int lbkt = content.indexOf("(",start);
        if(lbkt == -1)
            break;

        int newline = content.lastIndexOf("\n",lbkt - 1);
        if(newline == -1)
            newline = 0;

        int end = content.indexOf(";",lbkt);
        if(end == -1){
            qDebug() << "file error, missing ;";
            break;
        }
        start = end + 1;

        QString funcDefine = content.mid(newline,(end - 1 - newline) + 1).simplified();
        func_list << funcDefine;
    }

    for(int i = 0; i < func_list.size(); i++)
    {
        FunctionDefine func;
        func.parse(func_list[i]);
        results.push_back(func);
    }

    return results;
}
