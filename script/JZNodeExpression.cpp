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
#include "angelscript/as_parser.h"
#include "JZScriptConvert.h"

//JZNodeExpression
JZNodeExpression::JZNodeExpression()
{
    m_type = Node_expr;
    m_name = "expr";
    m_exprItem = new JZScriptItem(ProjectItem_scriptFunction);    
}

JZNodeExpression::~JZNodeExpression()
{
    delete m_exprItem;
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

bool JZNodeExpression::updateExpr(QString &error)
{    
    auto project = m_file->project();
    JZTempItemGuard guard(project, m_exprItem, true);        
    m_convert.init(m_exprItem);
    if (!m_convert.convertExpression(m_expression))
    {
        error = m_convert.error();
        return false;
    }

    JZScriptInOutInfo result;
    JZNodeCompiler c;
    if (!c.genNodeInputOuput(m_exprItem, result))
    {
        error = c.error();
        return false;
    }
    
    for (int i = 0; i < result.inList.size(); i++)
    {
        int id = addParamIn(result.inList[i]);        
        setPinTypeNumber(id);
    }
    for (int i = 0; i < result.outList.size(); i++)
    {
        int id = addParamOut(result.outList[i]);
        setPinTypeNumber(id);
    }
    return true;
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

bool JZNodeExpression::compiler(JZNodeCompiler *c,QString &error)
{                
    if(!updateExpr(error))
        return false;
        
    if(!c->addDataInput(m_id,error))
        return false;
    
    QVector<GraphPtr> graph_list;
    JZNodeCompiler tmp_c;
    if (!tmp_c.genGraphs(m_exprItem, graph_list))
        return false;

    
    GraphPtr graph = graph_list[0];    
    for (int expr_idx = 0; expr_idx < graph->topolist.size(); expr_idx++)
    {
        JZNode* node = graph->topolist[expr_idx]->node;
        int node_type = node->type();
        if (node_type == Node_functionStart)
        {

        }
        else if (node_type >= Node_add && node_type <= Node_or) //func
        {
            
        }
        else if (node_type == Node_bitresver || node_type == Node_not)        
        {
            
        }
        else if (node_type == Node_function)
        {

        }
        else
        {
            Q_ASSERT(0);
        }

    }
    return true;
}
