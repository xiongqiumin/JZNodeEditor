#ifndef JZ_EXPRESSION_H_
#define JZ_EXPRESSION_H_

#include <QStringList>

class JZExpression
{
public:
    JZExpression();
    bool parse(const QString &expr,QString &error);

    QStringList inList;
    QStringList outList;
};



#endif