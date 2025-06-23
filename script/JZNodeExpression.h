#ifndef JZNODE_EXPRESSION_H_
#define JZNODE_EXPRESSION_H_

#include "JZNodeOperator.h"

//JZNodeExpression
class JZNodeExpression: public JZNodeOperator
{
public:
    JZNodeExpression();
    ~JZNodeExpression();

    void setExpr(QString expr);
    QString expr();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    virtual bool updateNode(QString &error) override;

protected:
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    
    QString m_expression;
    JZScriptItem *m_exprItem;    
};

#endif
