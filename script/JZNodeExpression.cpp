#include <QRegularExpression>
#include <QPushButton>
#include <QScopeGuard>
#include <QDebug>
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
    m_exprItem = new JZScriptItem(JZScriptItem::Function);
    m_expression = "c = a + b;";
}

JZNodeExpression::~JZNodeExpression()
{
    delete m_exprItem;
}

void JZNodeExpression::setExpr(QString expr)
{
    m_expression = expr;
    update();
}

QString JZNodeExpression::expr()
{
    return m_expression;
}

bool JZNodeExpression::updateNode(QString &error)
{    
    auto project = m_file->project();

    JZProjectTempGuard guard(project, m_exprItem, JZProjectTempGuard::TakeItem);

    JZScriptConvert convert;
    if (!convert.convertExpression(m_expression,m_exprItem))
    {
        error = convert.error();
        return false;
    }

    JZScriptInOutInfo result;
    JZNodeCompiler c;
    if (!c.genNodeInputOuput(m_exprItem, result))
    {
        error = "get input ouput failed";
        return false;
    }
    
    clearPin();
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
    if(!c->addDataInput(m_id,error))
        return false;

    auto env = environment();
    
    auto project = m_file->project();
    JZProjectTempGuard guard(project, m_exprItem, JZProjectTempGuard::TakeItem);
    m_exprItem->clearLocalVariable();

    QMap<QString, int> localMap;
    auto in_list = paramInList(); 
    for (int i = 0; i < in_list.size(); i++)
    {
        int pin_id = in_list[i];
        m_exprItem->addLocalVariable(pinName(pin_id),env->typeToName(c->pinType(m_id,pin_id)));
        localMap[pinName(pin_id)] = c->paramId(m_id, pin_id);
    }

    auto out_list = paramOutList();
    Q_ASSERT(out_list.size() == 1);

    int out_id = out_list[0];
    m_exprItem->addLocalVariable(pinName(out_id), "auto");
    localMap[pinName(out_id)] = c->paramId(m_id, out_id);
    QString out_name = pinName(out_id);
    
    JZNodeScript script;

    JZNodeCompiler tmp_c;
    if (!tmp_c.build(m_exprItem, &script))
    {
        error = "gen express failed";
        return false;
    }

    QList<JZNodeIRPtr> ir_ref_list;
    QList<JZNodeIRPtr> ir_list = script.statmentList;
    for (int i = 0; i < ir_list.size(); i++)
    {        
        if (ir_list[i].data()->type == OP_reference)
            ir_ref_list << ir_list[i];
        if (ir_list[i].data()->type == OP_nodeEnter)
        {
            ir_list = ir_list.mid(i);
            break;
        }
    }
    ir_list.pop_back();

    c->setPinType(m_id,paramOut(0), tmp_c.refType(out_name));
    int pre_stack_id = c->stackId();

    JZMacroIRReplace replace(&tmp_c);
    replace.setLocalMap(localMap);
    replace.setStackId(c->stackId());
    replace.replace(ir_list);

    for (int i = pre_stack_id; i < replace.stackId(); i++)
    {
        c->allocStack(replace.stackType(i));
    }
    for (int i = 0; i < ir_ref_list.size(); i++)
    {
        JZNodeIRReference *ir_ref = dynamic_cast<JZNodeIRReference*>(ir_ref_list[i].data());
        JZNodeIRParam ref = ir_ref->ref;
        JZNodeIRParam orig = ir_ref->orig;
        replace.replaceIr(ref);
        replace.replaceIr(orig);
        c->setIRParamReference(ref, orig);
    }
    Q_ASSERT(c->stackId() == replace.stackId());

    ir_list.front()->memo = "macro begin";
    ir_list.back()->memo = "macro end";
    c->appendStatementList(ir_list);
    return true;
}
