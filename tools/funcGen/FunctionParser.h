#ifndef FUNCTION_PARSER_H_
#define FUNCTION_PARSER_H_

#include <QString>
#include <QList>
#include <QFile>

class ParamDefine
{
public:
    void parse(QString text);
    QString defineType();

    QString name;
    QString type;
    QString defaultValue;
};

class FunctionDefine
{
public:
    void parse(QString line);
    QString pointDeclare();
    QString enumDeclare();
    bool hasReturn();

    QString name;
    ParamDefine result;
    QList<ParamDefine> paramList;
};

class FunctionParser
{

public:
    FunctionParser();
    ~FunctionParser();
    QList<FunctionDefine> parse(QString filepath);

};



#endif
