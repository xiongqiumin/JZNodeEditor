#include <QDebug>
#include <QStack>
#include "JZProject.h"
#include "JZScriptConvert.h"
#include "JZNodeFactory.h"
#include "JZNodeEvent.h"
#include "angelscript/as_parser.h"
#include "angelscript/as_scriptcode.h"
#include "JZNodeOperator.h"
#include "JZNodeValue.h"
#include "JZNodeFunction.h"

JZScriptConvert::Flow::Flow()
{
    index = 0;
}

JZScriptConvert::JZScriptConvert()
{
    m_script = nullptr;	
}

JZScriptConvert::~JZScriptConvert()
{
}

JZScriptEnvironment* JZScriptConvert::environment()
{
	return m_script->project()->environment();
}

void JZScriptConvert::init(JZScriptItem* item)
{
    m_script = item;
    m_flowStack.clear();

    Flow f;
    f.nodes.push_back(item->startNode());
}

QString JZScriptConvert::error()
{
    return m_error;
}

bool JZScriptConvert::convertFunction(QString code)
{
    asCScriptCode script;
    script.SetCode(m_script->itemPath(), code);
	m_code = code;

    asCParser parser;
    int ret = parser.ParseFunction(&script);
    if (ret != 0)
    {
        m_error = parser.Error();
        return false;
    }

    auto root = parser.GetScriptNode();
    return updateFunction(root);
}

bool JZScriptConvert::convertStatments(QString code)
{
    asCScriptCode script;
    script.SetCode(m_script->itemPath(), code);
	m_code = code;

    asCParser parser;
    int ret = parser.ParseFunctionStatement(&script);
    if (ret != 0)
    {
        m_error = parser.Error();
        return false;
    }

	m_script->clear();

	auto root = parser.GetScriptNode();
	auto node_start = m_script->getNode(0);
	auto jz_node_list = toStatementBlock(root);
	if (jz_node_list.size() != 0)
		m_script->addConnect(node_start->flowOutGemo(), jz_node_list[0]->flowInGemo());

	return true;
}

bool JZScriptConvert::convertLambda(QString code, JZFunctionDefine& define)
{
	if (convertStatments(code))
		return false;

	return true;
}

bool JZScriptConvert::updateFunction(asCScriptNode* node)
{
    JZFunctionDefine func;

    auto child = node->firstChild;

    auto func_ret = nodeText(child);
    if (func_ret != "void")
        func.paramOut.push_back(JZParamDefine("output", func_ret));

    child = nextNode(child, 2);

    func.name = nodeText(child);
    child = nextNode(child, 1);

    func.paramIn = toParamList(child);
    child = nextNode(child, 1);
    
    m_script->setFunction(func);
    m_script->clear();

	auto node_start = m_script->getNode(0);
	auto jz_node_list = toStatementBlock(child);
	if (jz_node_list.size() != 0)
		m_script->addConnect(node_start->flowOutGemo(), jz_node_list[0]->flowInGemo());

    return true;
}

void JZScriptConvert::nodeDebug(asCScriptNode* root, QString& result, int level)
{
	QString type = asCScriptNode::GetDefinition(root->nodeType);
	QString space = QString().leftJustified(level * 2);

	result += space + "[" + type + "] " + nodeText(root) + "\n";

	if (root->firstChild)
		result += space + "{\n";

	auto child = root->firstChild;
	while (child)
	{
		nodeDebug(child, result, level + 1);
		child = child->next;
	}

	if (root->firstChild)
		result += space + "}\n";
}

QString JZScriptConvert::nodeDebug(asCScriptNode* node)
{
	QString line;
	nodeDebug(node, line, 0);
	return line;
}

void JZScriptConvert::printNode(asCScriptNode* node)
{
	qDebug().noquote() << nodeDebug(node);
}

QString JZScriptConvert::nodeText(asCScriptNode* node)
{
	QString text = m_code.mid(node->tokenPos, node->tokenLength);
	if (text.indexOf("\n") >= 0)
	{
		int idx = text.indexOf("\n");
		text = text.left(idx);
		text += "...";
	}
	return text;
}

asCScriptNode* JZScriptConvert::nextNode(asCScriptNode* node, int count)
{
    for (int i = 0; i < count; i++)
    {
        node = node->next;
        if (!node)
            return nullptr;
    }
    return node;
}

QList<asCScriptNode*> JZScriptConvert::childList(asCScriptNode* node)
{
    QList<asCScriptNode*> list;
    auto child = node->firstChild;
    while (child)
    {
        list.push_back(child);
        child = child->next;
    }
    return list;
}

QList<JZParamDefine> JZScriptConvert::toParamList(asCScriptNode* node)
{
    QList<JZParamDefine> list;

    auto child = node->firstChild;
    while (child)
    {
        JZParamDefine p;
        if (child->nodeType == snDataType)
        {
            p.type = nodeText(child);
            child = nextNode(child, 2);
        }

        if (child && child->nodeType == snIdentifier)
        {
            p.name = nodeText(child);
            child = nextNode(child, 1);
        }

        list.push_back(p);
    }
    return list;
}

JZNode *JZScriptConvert::toStatement(asCScriptNode* node)
{
    int node_type = node->nodeType;
    if (node_type == snReturn)
        return toReturn(node);
    else if (node_type == snIf)
        return toIf(node);
    else if (node_type == snFor)
        return toFor(node);
    else if (node_type == ttWhile)
        return toWhile(node);
    else if (node_type == ttBreak)
        return toBreak(node->firstChild);
    else if (node_type == ttContinue)
        return toContinue(node->firstChild);
    else if (node_type == ttSwitch)
        return toSwitch(node->firstChild);
    else if (node_type == snFunctionCall)
        return toFunctionCall(node);
    else if (node_type == snExpressionStatement)
        return toAssignment(node->firstChild);
    else
    {
        qDebug() << node_type;
        Q_ASSERT_X(0, "Error", qUtf8Printable(nodeText(node)));
        return nullptr;
    }
}

JZNode* JZScriptConvert::createOpNode(QString op)
{
	if (op == "+")
		return createNode<JZNodeAdd>();
	else if (op == "-")
		return createNode<JZNodeSub>();
	else if (op == "*")
		return createNode<JZNodeMul>();
	else if (op == "/")
		return createNode<JZNodeDiv>();
	else if (op == "%")
		return createNode<JZNodeMod>();
	else if (op == "&")
		return createNode<JZNodeBitAnd>();
	else if (op == "^")
		return createNode<JZNodeBitXor>();
	else if (op == "|")
		return createNode<JZNodeBitOr>();
	else if (op == "<=")
		return createNode<JZNodeLE>();
	else if (op == "<")
		return createNode<JZNodeLT>();
	else if (op == ">=")
		return createNode<JZNodeGE>();
	else if (op == ">")
		return createNode<JZNodeGT>();
	else if (op == "==")
		return createNode<JZNodeEQ>();
	else if (op == "!=")
		return createNode<JZNodeNE>();
	else if (op == "&&")
		return createNode<JZNodeAnd>();
	else if (op == "||")
		return createNode<JZNodeOr>();

	Q_ASSERT(0);
	return nullptr;
}

JZNode* JZScriptConvert::toReturn(asCScriptNode* node)
{
	Q_ASSERT(node->nodeType == snReturn);

	JZNodeReturn* ret = createNode<JZNodeReturn>();
	ret->setFunction(&m_script->function());
	if (node->firstChild)
	{
		JZNode* data = toAssignment(node->firstChild);
		if (!data)
			return nullptr;

		m_script->addConnectForce(data->paramOutGemo(0), ret->paramInGemo(0));
	}
	return ret;
}

void JZScriptConvert::setNodeIf(JZNodeIf* node_if, asCScriptNode* as_node, int cond)
{
	Q_ASSERT(as_node->nodeType == snIf);
	auto child = childList(as_node);

	if (node_if->subFlowCount() <= cond)
		node_if->addCondPin();

	auto jz_cond = toAssignment(child[0]);
	m_script->addConnectForce(jz_cond->paramOutGemo(0), node_if->paramInGemo(cond));

	QList<JZNode*> stmt = toStatementBlock(child[1]);
	m_script->addConnect(node_if->subFlowOutGemo(cond), stmt[0]->flowInGemo());

	if (child.size() > 2)
	{
		if (child[2]->nodeType == snIf)
			setNodeIf(node_if, child[2], cond + 1);
		else
		{
			node_if->addElsePin();
			QList<JZNode*> else_stmt = toStatementBlock(child[2]);
			int else_index = node_if->subFlowCount() - 1;
			m_script->addConnect(node_if->subFlowOutGemo(else_index), else_stmt[0]->flowInGemo());
		}
	}
}

JZNode* JZScriptConvert::toIf(asCScriptNode* as_node)
{
	JZNodeIf* node_if = createNode<JZNodeIf>();
	setNodeIf(node_if, as_node, 0);
	return node_if;
}

JZNode* JZScriptConvert::toFor(asCScriptNode* node)
{
	return nullptr;
}

JZNode* JZScriptConvert::toAssignment(asCScriptNode* node)
{
	Q_ASSERT(node->nodeType == snAssignment);

	auto list = childList(node);
	if (list.size() == 1)
		return toExpression(list[0]->firstChild);
	else if (list.size() == 3)
	{
		if (list[1]->nodeType == snExprOperator)
		{
			QString op = nodeText(list[1]);
			if (op == "=")
			{
				auto node_set = createNode<JZNodeSetParam>();
				node_set->setVariable(nodeText(list[0]));

				auto node_value = toAssignment(list[2]);
				m_script->addConnectForce(node_value->paramOutGemo(0), node_set->paramInGemo(1));
				return node_set;
			}
		}
	}

	printNode(node);
	Q_ASSERT(0);
	return nullptr;
}

JZNode* JZScriptConvert::toFunctionCall(asCScriptNode* node)
{
	Q_ASSERT(node->nodeType == snFunctionCall);

	auto func_inst = environment()->functionManager();
	auto node_name = node->firstChild;
	JZNodeFunction* func = createNode<JZNodeFunction>();
	func->setFunction(func_inst->function(nodeText(node_name)));

	auto arg_list = childList(node_name->next);
	for (int i = 0; i < arg_list.size(); i++)
	{
		auto param = toAssignment(arg_list[i]);
		m_script->addConnectForce(param->paramOutGemo(0), func->paramInGemo(i));
	}

	return func;
}

JZNode* JZScriptConvert::toExprTerm(asCScriptNode* root)
{
	Q_ASSERT(root->nodeType == snExprTerm);

	auto env = environment();
	JZNode* ret = nullptr;
	auto node_value = root->firstChild;
	auto node = node_value->firstChild;
	if (node->nodeType == snConstant)
	{
		QString value = nodeText(node);
		auto literal = createNode<JZNodeLiteral>();
		literal->setDataType(env->stringType(value));
		literal->setLiteral(value);
		ret = literal;
	}
	else if (node->nodeType == snAssignment)
	{
		ret = toAssignment(node);
	}
	else if (node->nodeType == snVariableAccess)
	{
		auto param = createNode<JZNodeParam>();
		param->setVariable(nodeText(node));
		ret = param;
	}
	else if (node->nodeType == snFunctionCall)
	{
		ret = toFunctionCall(node);
	}
	else
	{
		Q_ASSERT(0);
	}

	return ret;
}

JZNode* JZScriptConvert::toWhile(asCScriptNode* node)
{
	JZNodeWhile* node_while = createNode<JZNodeWhile>();
	return node_while;
}

JZNode* JZScriptConvert::toSwitch(asCScriptNode* node)
{
	JZNodeSwitch* node_switch = createNode<JZNodeSwitch>();
	return node_switch;
}

JZNode* JZScriptConvert::toBreak(asCScriptNode* node)
{
	JZNodeBreak* node_break = createNode<JZNodeBreak>();
	return node_break;
}

JZNode* JZScriptConvert::toContinue(asCScriptNode* node)
{
	JZNodeContinue* node_continue = createNode<JZNodeContinue>();
	return node_continue;
}

QList<JZNode*> JZScriptConvert::toStatementBlock(asCScriptNode* node)
{
	QList<JZNode*> list;
	if (node->nodeType != snStatementBlock)
	{
		list.push_back(toStatement(node));
		return list;
	}

	JZNode* pre = nullptr;
	auto child = node->firstChild;
	while (child)
	{
		JZNode* jz_node = toStatement(child);
		if (!jz_node)
			return QList<JZNode*>();

		if (pre)
			m_script->addConnect(pre->flowOutGemo(), jz_node->flowInGemo());
		pre = jz_node;
		list.push_back(jz_node);
		child = nextNode(child, 1);
	}

	return list;
}

JZNode* JZScriptConvert::toExpression(asCScriptNode* node)
{
	struct ExprToken
	{
		enum {
			Op,
			Value,
		};

		bool isOp() const
		{
			return (type == Op);
		}

		int type;
		JZNode* node;
	};
	Q_ASSERT(node->nodeType == snExpression);

	auto list = childList(node);
	auto opPri = [](const ExprToken& tk)->int {
		auto* op_node = dynamic_cast<const JZNodeOperator*>(tk.node);
		QString str_op = JZNodeType::opName(op_node->op());
		return JZNodeType::opPri(str_op);
		};

	QStack<ExprToken> jz_stack;
	QList<ExprToken> token_expr;
	for (int i = 0; i < list.size(); i++)
	{
		auto sub_node = list[i];
		if (sub_node->nodeType == snExprTerm)
		{
			ExprToken tk;
			tk.type = ExprToken::Value;
			tk.node = toExprTerm(sub_node);
			token_expr.push_back(tk);
		}
		else if (sub_node->nodeType == snExprOperator)
		{
			QString op = nodeText(sub_node);

			int pri = JZNodeType::opPri(op);
			int top_pri = jz_stack.size() > 0 ? opPri(jz_stack.top()) : -1;
			if (pri <= top_pri)
			{
				while (jz_stack.size() > 0 && jz_stack.top().isOp())
				{
					top_pri = opPri(jz_stack.top());
					if (pri <= top_pri)
						token_expr.push_back(jz_stack.pop());
					else
						break;
				}
			}

			ExprToken tk;
			tk.type = ExprToken::Op;
			tk.node = createOpNode(op);
			jz_stack.push(tk);
		}
		else
		{
			Q_ASSERT(0);
		}
	}
	while (jz_stack.size() > 0)
	{
		token_expr.push_back(jz_stack.pop());
	}

	// calc
	for (int i = 0; i < token_expr.size(); i++)
	{
		if (token_expr[i].isOp())
		{
			auto b = jz_stack.pop();
			auto a = jz_stack.pop();

			m_script->addConnectForce(a.node->paramOutGemo(0), token_expr[i].node->paramInGemo(0));
			m_script->addConnectForce(b.node->paramOutGemo(0), token_expr[i].node->paramInGemo(1));

			ExprToken tk;
			tk.type = ExprToken::Value;
			tk.node = token_expr[i].node;
			jz_stack.push(tk);
		}
		else
			jz_stack.push(token_expr[i]);
	}

	Q_ASSERT(jz_stack.size() == 1);
	return jz_stack[0].node;
}