#ifndef JZNODE_EXPRESSION_H_
#define JZNODE_EXPRESSION_H_

#include "JZNode.h"
#include "JZScriptConvert.h"

//JZNodeExpression
class JZNodeExpression: public JZNode
{
public:
    JZNodeExpression();
    ~JZNodeExpression();

    bool setExpr(QString expr,QString &error);
    QString expr();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    QStringList irList();

protected:
    struct VarInfo
    {
        int type;
        int stackId;
    };

    bool updateExpr(QString &error);
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    QString m_expression;
    JZScriptItem *m_exprItem;    
    JZScriptConvert m_convert;
};

#endif
