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

bool JZRegExpHelp::isNumber(const QString &str)
{
    return isInt(str) || isFloat(str) || isHex(str);
}

bool JZRegExpHelp::isBool(const QString &str)
{    
    return (str == "true" || str == "false");
}

bool JZRegExpHelp::isWord(const QString &str)
{
    if(str.size() == 0)
        return false;

    QChar first = str[0];
    if(first >= '0' && first <= '9')
        return false;

    for(int i = 0; i < str.size(); i++)
    {
        QChar c = str[i];
    }

    return true;
}

bool JZRegExpHelp::isString(const QString &text)
{
    return (text.size() >= 2 && text.front() == '"' && text.back() == '"');
}

bool JZRegExpHelp::isIdentify(const QString &str)
{
    if (str.isEmpty())
        return false;

    return true;
}

QString JZRegExpHelp::uniqueString(const QString &text, const QStringList &list)
{
    QString result = text;
    int idx = 2;
    while(true)
    {
        if (!list.contains(result))
            return result;

        result = text + QString::number(idx);
        idx++;
    }

    return result;
}