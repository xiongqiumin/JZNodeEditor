#include "JZRegExpHelp.h"
#include <QRegularExpression>

bool JZRegExpHelp::isInt(const QString &str)
{
    QRegularExpression exp("^-?[0-9]+[0-9]*$");    
    return exp.match(str).hasMatch();
}

bool JZRegExpHelp::isHex(const QString &str)
{
    QRegularExpression exp("^0[xX][0-9a-fA-F]+$");
    return exp.match(str).hasMatch();
}

bool JZRegExpHelp::isFloat(const QString &str)
{
    QRegularExpression exp("^-?[0-9]+\\.[0-9]+$");
    return exp.match(str).hasMatch();
}

bool JZRegExpHelp::isBool(const QString &str)
{    
    return (str == "true" || str == "false");
}