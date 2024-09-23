#ifndef JZREG_EXP_HELP_H_
#define JZREG_EXP_HELP_H_

#include <QString>
#include "JZNodeCoreDefine.h"

class JZCORE_EXPORT JZRegExpHelp
{
public:
    static bool isBool(const QString &str);
    static bool isInt(const QString &str);
    static bool isHex(const QString &str);
    static bool isFloat(const QString &str);
    static bool isNumber(const QString &str);
    
    static bool isWord(const QString &str);
    static bool isString(const QString &str);
    static bool isIdentify(const QString &str);
    static QString uniqueString(const QString &text,const QStringList &list);
};


#endif // !JZREG_EXP_HELP_H_
