#include <QRegularExpression>
#include <QPushButton>
#include <QScopeGuard>
#include "JZNodeExpression.h"
#include "JZNodeIR.h"
#include "JZNodeCompiler.h"
#include "JZRegExpHelp.h"
#include "JZNodeValue.h"
#include "JZProject.h"
#include "JZNodeOperator.h"
#include "JZNodeFunction.h"

//JZNodeExpression
JZNodeExpression::JZNodeExpression()
{
    m_type = Node_expr;
    m_name = "expr";
    m_stackIdx = 0;
    m_compiler = nullptr;
}

bool JZNodeExpression::setExpr(QString expr,QString &error)
{
    Q_ASSERT(m_file);
    m_expression = expr;
    return updateExpr(error);
}

QString JZNodeExpression::expr()
{
    return m_expression;
}

QStringList JZNodeExpression::irList()
{
    return m_exprList;
}

bool JZNodeExpression::updateExpr(QString &error)
{    
    return false;
}

void JZNodeExpression::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
    s << m_expression;
}

void JZNodeExpression::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
    s >> m_expression;
}

JZNodeIRParam JZNodeExpression::toIr(const QString &name)
{
    auto env = environment();
    JZNodeIRParam parma;
    if (name.startsWith("#Reg"))
    {
        int reg_index = name.mid(4).toInt();
        Q_ASSERT(m_varMap.contains(reg_index));
        int id = m_varMap[reg_index].stackId;
        return irId(id);
    }
    else if(JZRegExpHelp::isWord(name))
    {
        auto ptr = pin(name);
        Q_ASSERT(ptr);
        int id = JZNodeCompiler::paramId(m_id, ptr->id());
        return irId(id);
    }
    else
    {
        int type = env->stringType(name);
        QVariant v = env->initValue(type,name); 
        return irLiteral(v);
    }
}

int JZNodeExpression::getIrType(const QString &name)
{
    auto env = environment();
    JZNodeIRParam parma;
    if (name.startsWith("#Reg"))
    {
        int reg_index = name.mid(4).toInt();
        Q_ASSERT(m_varMap.contains(reg_index));
        return m_varMap[reg_index].type;
    }
    else if(JZRegExpHelp::isWord(name))
    {
        auto ptr = pin(name);
        Q_ASSERT(ptr);
        if(ptr->isOutput())
        {
            Q_ASSERT(m_outType.contains(name));
            return m_outType[name];
        }
        else
        {
            return m_compiler->pinType(m_id,ptr->id());
        }
    }
    else
    {
        return env->stringType(name);
    }
}

void JZNodeExpression::setIrType(const QString &name,int type)
{
    auto env = environment();
    JZNodeIRParam parma;
    if (name.startsWith("#Reg"))
    {
        int reg_index = name.mid(4).toInt();
        if(m_varMap.contains(reg_index))
        {
            m_varMap[reg_index].type = env->upType(m_varMap[reg_index].type,type);
        }
        else
        {
            m_varMap[reg_index].stackId = m_stackIdx++;
            m_varMap[reg_index].type = type;
        }
    }
    else if(JZRegExpHelp::isWord(name))
    {
        auto ptr = pin(name);
        Q_ASSERT(ptr);
        if(ptr->isOutput())
        {
            if(m_outType.contains(name))
                m_outType[name] = env->upType(m_outType[name],type);
            else
                m_outType[name] = type;
        }
    }
    else
    {
        Q_ASSERT(0);
    }
}

bool JZNodeExpression::compiler(JZNodeCompiler *c,QString &error)
{            
    if(!updateExpr(error))
        return false;

    auto env = environment();
    m_compiler = c;
    if(!c->addDataInput(m_id,error))
        return false;
    
    m_stackIdx = 0;
    m_varMap.clear();

    //get param type
    for(int exp_idx = 0; exp_idx < m_exprList.size(); exp_idx++)
    {
        QStringList strs = m_exprList[exp_idx].split(" ");
        if(strs[2].startsWith("@")) //func
        {
            int s = strs[2].indexOf("(");

            QString function = strs[2].mid(1,s - 1);
            QStringList params = strs[2].mid(s + 1,strs[2].size()-1 - (s + 1)).split(",");
            const JZFunctionDefine *define = c->function(function);
            if(!define)
            {
                error = "no such function " + function;
                return false;
            }
            if(define->paramIn.size() != params.size())
            {
                error = QString("Function %0 pamram error, expert %1 give %2.").arg(function,
                    QString::number(define->paramIn.size()),QString::number(params.size()));
                return false;
            }
            if(define->paramOut.size() != 1)
            {
                error = QString("Function %0 return error").arg(function);
                return false;
            }
            setIrType(strs[0], env->nameToType(define->paramOut[0].type));
        }
        else
        {
            int dst_type;
            if(strs.size() == 5)
            {
                int t1 = getIrType(strs[2]);
                int t2 = getIrType(strs[4]);
                int op = JZNodeType::opType(strs[3]);
                dst_type = JZNodeType::calcExprType(t1,t2,op);
            }
            else
            {
                int t1 = getIrType(strs[2]);
                dst_type = t1;
            }
            setIrType(strs[0],dst_type);
        }
    }

    //alloc
    for(auto &var : m_varMap)
        var.stackId = c->allocStack(var.type);
    
    auto out_it = m_outType.begin();
    while (out_it != m_outType.end())
    {
        int out_id = indexOfPinByName(out_it.key());
        c->setPinType(m_id,out_id, out_it.value());
        out_it++;
    }

    //calc
    for(int expr_idx = 0; expr_idx < m_exprList.size(); expr_idx++)
    {
        QStringList strs = m_exprList[expr_idx].split(" ");
        if(strs[2].startsWith("@")) //func
        {
            int s = strs[2].indexOf("(");
            QString function = strs[2].mid(1,s - 1);
            QStringList params = strs[2].mid(s + 1,strs[2].size()-1 - (s + 1)).split(",");
        
            QList<JZNodeIRParam> paramIn;            
            for(int i = 0; i < params.size(); i++)
                paramIn << toIr(params[i]);
                
            QList<JZNodeIRParam> paramOut;            
            paramOut << toIr(strs[0]);
            c->addCallConvert(function,paramIn,paramOut);
        }
        else
        {
            if(strs.size() == 5)
            {
                c->addExprConvert(toIr(strs[0]),toIr(strs[2]),toIr(strs[4]),JZNodeType::opType(strs[3]));
            }
            else
            {
                c->addSetVariableConvert(toIr(strs[0]),toIr(strs[2]));
            }
        }
    }

    return true;
}
