#ifndef JZREG_EXP_HELP_H_
#define JZREG_EXP_HELP_H_

#include <QString>

class JZRegExpHelp
{
public:
    static bool isInt(const QString &str);
    static bool isHex(const QString &str);
    static bool isFloat(const QString &str);
    static bool isBool(const QString &str);
    static bool isWord(const QString &str);
    static bool isString(const QString &str);
    static bool isIdentify(const QString &str);
    static QString uniqueString(const QString &text,const QStringList &list);
};


#endif // !JZREG_EXP_HELP_H_
