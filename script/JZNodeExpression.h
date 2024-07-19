#ifndef JZNODE_EXPRESSION_H_
#define JZNODE_EXPRESSION_H_

#include "JZNode.h"

//JZNodeExpression
class JZNodeExpression: public JZNode
{
public:
    JZNodeExpression();

    bool setExpr(QString expr,QString &error);
    QString expr();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

protected:
    struct VarInfo
    {
        int type;
        int stackId;
    };

    bool updateExpr(QString &error);
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    JZNodeIRParam toIr(const QString &name);

    int getIrType(const QString &name);
    void setIrType(const QString &name,int type);

    QString m_expression;
    QStringList m_exprList;

    QMap<int,VarInfo> m_varMap;
    QMap<QString,int> m_outType;
    int m_stackIdx;
    JZNodeCompiler *m_compiler;
};

#endif
