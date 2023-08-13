#ifndef JZREG_EXP_HELP_H_
#define JZREG_EXP_HELP_H_

#include <QString>

class JZRegExpHelp
{
public:
    static bool isInt(const QString &str);
    static bool isHex(const QString &str);
    static bool isFloat(const QString &str);
};


#endif // !JZREG_EXP_HELP_H_
