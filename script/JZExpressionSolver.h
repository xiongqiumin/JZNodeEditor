#ifndef JZ_EXPRESSION_SOLVER_H_
#define JZ_EXPRESSION_SOLVER_H_

#include <QString>
#include "JZNodeProgram.h"

QString removeBrackets(QString input);
QString expressionSolver(const QString &expr);

class JZExpressRunner
{
public:
    bool init(QString expr, QString &error, QMap<QString,int> typeMap);
    bool call(const QVariantList &in, QVariantList &out);

protected:
    QString expr;
    JZNodeScriptPtr m_script;
};

void test_solver();






#endif