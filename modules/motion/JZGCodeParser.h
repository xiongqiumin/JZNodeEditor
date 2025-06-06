#ifndef JZGCODEPARSER_H
#define JZGCODEPARSER_H

#include <QString>
#include <QList>
#include "JZGCodeProgram.h"

class JZGCodeParser 
{

public:
    JZGCodeParser();
    ~JZGCodeParser();

    bool parse(const QString &gcode, JZGCodeProgram& program);

protected:
    // 可添加解析所需的成员变量

};

#endif // JZGCODEPARSER_H