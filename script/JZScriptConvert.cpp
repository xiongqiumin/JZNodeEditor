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
#include "JZNodeCompiler.h"
#include "JZScriptItemVisitor.h"

JZScriptConvert::BlockEnv::BlockEnv()
{
	preStatment = nullptr;
	postStatment = nullptr;
}

//JZScriptConvert
JZScriptConvert::JZScriptConvert()
{
    m_script = nullptr;	
}

JZScriptConvert::~JZScriptConvert()
{
}

void JZScriptConvert::init(JZScriptItem* script)
{
	m_script = script;
	m_script->clear();
	m_blockEnv.clear();
}

JZScriptEnvironment* JZScriptConvert::environment()
{
	return m_script->project()->environment();
}

const JZFunctionDefine* JZScriptConvert::function(QString name)
{
	return JZNodeCompiler::function(m_script, name);
}

const JZParamDefine* JZScriptConvert::getVariableInfo(QString name)
{
	for (int i = m_blockEnv.size() - 1; i >= 0; i--)
	{
		auto b = m_blockEnv[i].data();
		for (int param_idx = 0; param_idx < b->paramList.size(); param_idx++)
		{
			if (b->paramList[param_idx].name == name)
			{
				QString param_name = b->paramList[param_idx].name;
				Q_ASSERT(m_localVaribaleMap.contains(param_name));
				
				return &m_localVaribaleMap[param_name];
			}
		}
	}

	return JZNodeCompiler::getVariableInfo(m_script, name);
}

void JZScriptConvert::addLocalVariable(QString name, QString data_type)
{
	currentBlock()->paramList.push_back(JZParamDefine(name, data_type));

	QStringList variables;
	for (auto& v : m_localVaribaleMap)
		variables << v.name;

	QString replace_name;
	if (!m_localVaribaleMap.contains(name))
		replace_name = name;
	else
		replace_name = JZRegExpHelp::uniqueString("__tmp__" + name, variables);
	
	JZParamDefine define;
	define.name = replace_name;
	define.type = data_type;
	m_localVaribaleMap.insert(name, define);
	m_script->addLocalVariable(replace_name, data_type);
}

QString JZScriptConvert::error()
{
    return m_error;
}

void JZScriptConvert::visitNode(asCScriptNode* node,std::function<void(asCScriptNode*)> vistor)
{
	while (node)
	{
		if (node->firstChild)
			visitNode(node->firstChild, vistor);

		vistor(node);
		node = node->next;
	}
}

bool JZScriptConvert::convertScript(QString code, JZScriptFile* file)
{
	asCScriptCode script;
	script.SetCode(file->itemPath(), code);
	m_code = code;

	asCParser parser;
	int ret = parser.ParseScript(&script);
	if (ret != 0)
	{
		m_error = parser.Error();
		return false;
	}

	auto root = parser.GetScriptNode();
	auto child = root->firstChild;
	while (child)
	{
		if (child->nodeType == snFunction)
		{
			JZFunctionDefine define;
			define.name = "newFunciton";

			JZScriptItem * jz_script = file->addFunction(define);
			init(jz_script);
			if (!addFunction(child))
				return false;
		}
		child = child->next;
	}

	return true;
}

bool JZScriptConvert::convertFunction(QString code, JZScriptItem* jz_script)
{
    asCScriptCode script;
    script.SetCode(jz_script->itemPath(), code);
	m_code = code;

    asCParser parser;
    int ret = parser.ParseFunction(&script);
    if (ret != 0)
    {
        m_error = parser.Error();
        return false;
    }
	init(jz_script);

    auto root = parser.GetScriptNode();    
    return addFunction(root);
}

bool JZScriptConvert::convertStatments(QString code, JZScriptItem* jz_script)
{
    asCScriptCode script;
    script.SetCode(jz_script->itemPath(), code);
	m_code = code;

    asCParser parser;
    int ret = parser.ParseFunctionStatement(&script);
    if (ret != 0)
    {
        m_error = parser.Error();
        return false;
    }
	init(jz_script);

	auto root = parser.GetScriptNode();
	auto node_start = m_script->getNode(0);

	pushBlock();
	QList<JZNode*> jz_node_list;
	if (!toStatementBlock(root, jz_node_list))
		return false;

	popBlock();
	m_script->addConnect(node_start->flowOutGemo(), jz_node_list[0]->flowInGemo());
	return true;
}

bool JZScriptConvert::convertExpression(QString code, JZScriptItem* jz_script)
{   
	m_script = jz_script;
    if (!code.trimmed().endsWith(";"))
        code += ";";

    asCScriptCode script;
    script.SetCode(jz_script->itemPath(), code);
	m_code = code;

    asCParser parser;
    int ret = parser.ParseFunctionStatement(&script);
    if (ret != 0)
    {
        m_error = parser.Error();
        return false;
    }

    auto node = parser.GetScriptNode();
	auto node_list = nodeChilds(node);
	if (node_list.size() != 1 || node_list[0]->nodeType != snExpressionStatement || !node_list[0]->firstChild
		|| node_list[0]->firstChild->nodeType != snAssignment)
	{
		m_error = "use as a = b + c;";
		return false;
	}
	init(jz_script);

	node = node_list[0]->firstChild;
	
	QStringList param_list;
	auto visitParma = [this,&param_list](asCScriptNode *node) 
	{
		if (node->nodeType == snVariableAccess)
			param_list << nodeText(node);
	};
	visitNode(node, visitParma);
	for (int i = 0; i < param_list.size(); i++)
		m_script->addLocalVariable(param_list[i], "double");
	
	pushBlock();
    auto jz_node = toAssignment(node);
	if (!jz_node)
		return false;
	popBlock();
	auto node_start = m_script->getNode(0);
    m_script->addConnect(node_start->flowOutGemo(), jz_node->flowInGemo());
    return true;
}

bool JZScriptConvert::addFunction(asCScriptNode* node)
{
    JZFunctionDefine func;
	func.isFlowFunction = false;

    auto child = node->firstChild;
	
    auto func_ret = toDataType(child);
    if (func_ret != "void")
        func.paramOut.push_back(JZParamDefine("output", func_ret));

    child = nextNode(child, 2);

    func.name = nodeText(child);
    child = nextNode(child, 1);

    func.paramIn = toParamList(child);
    child = nextNode(child, 1);    

    m_script->setFunction(func);    

	auto root = child;
	QList<JZNode*> jz_node_list;
	pushBlock();
	if (!toStatementBlock(root, jz_node_list))
		return false;
	popBlock();
	auto node_start = m_script->getNode(0);
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
	Q_ASSERT(node->tokenPos >= 0);

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

QList<asCScriptNode*> JZScriptConvert::nodeChilds(asCScriptNode* node)
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

QString JZScriptConvert::toDataType(asCScriptNode* child)
{
	Q_ASSERT(child->nodeType == snDataType);
	
	QString data_type;

	auto type_list = nodeChilds(child);
	if (type_list.size() == 1)
		data_type = nodeText(type_list[0]);
	else
	{
		data_type = nodeText(type_list[0]) + "<" + nodeText(type_list[1]) + ">";
	}
	child = nextNode(child, 1);
	if (child->firstChild)
	{
		QString post = nodeText(child->firstChild);
		Q_ASSERT(post == "&");
		data_type += "*";
	}
	return data_type;
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
			auto type_list = nodeChilds(child);
			if(type_list.size() == 1)
				p.type = nodeText(type_list[0]);
			else
			{
				p.type = nodeText(type_list[0]) + "<" + nodeText(type_list[1]) + ">";
			}
            child = nextNode(child, 1);
        }
		if (child->nodeType == snDataType && child->firstChild)
		{
			QString post = nodeText(child->firstChild);
			Q_ASSERT(post == "&");
			p.type += "*";
		}
		child = child->next;
        if (child && child->nodeType == snIdentifier)
        {
            p.name = nodeText(child);
            child = nextNode(child, 1);
        }

        list.push_back(p);
    }
    return list;
}

bool JZScriptConvert::toStatement(asCScriptNode* node)
{    
    int node_type = node->nodeType;
    if (node_type == snReturn)
		return toReturn(node);
    else if (node_type == snIf)
		return toIf(node);
    else if (node_type == snFor)
		return toFor(node);
    else if (node_type == snWhile)
		return toWhile(node);
    else if (node_type == snBreak)
		return toBreak(node->firstChild);
    else if (node_type == snContinue)
		return toContinue(node->firstChild);
    else if (node_type == snSwitch)
		return toSwitch(node);
    else if (node_type == snFunctionCall)
		return toFunctionCallStatement(node);
	else if (node_type == snExpressionStatement)
	{
		JZNode* jz_node = toExpressionStatementFlow(node);
		if (!jz_node)
			return false;

		currentBlock()->flowList << jz_node;
		return true;
	}
    else if (node_type == snDeclaration)
		return toDeclarationStatement(node);
    else
    {
        qDebug() << node_type;
        Q_ASSERT_X(0, "Error", qUtf8Printable(nodeText(node)));
        return false;
    }
}

JZNode* JZScriptConvert::createSingleOpNode(QString op)
{
	if (op == "-")
		return createNode<JZNodeNeg>();
	else if (op == "!")
		return createNode<JZNodeNot>();
	else if (op == "~")
		return createNode<JZNodeBitReverse>();

	Q_ASSERT(0);
	return nullptr;
}

JZNode* JZScriptConvert::createObject(QString type)
{
	JZNodeCreateObject* create = createNode<JZNodeCreateObject>();
	create->setClassName(type);
	return create;
}

JZNodeParam* JZScriptConvert::createGetParam(QString name)
{
	auto info = getVariableInfo(name);
	if (!info)
	{
		m_error = "no such variable " + name;
		return nullptr;
	}

	JZNodeParam* node = createNode<JZNodeParam>();
	node->setVariable(info->name);
	return node;
}

JZNodeSetParam* JZScriptConvert::createSetParam(QString name)
{
	auto info = getVariableInfo(name);
	if (!info)
	{
		m_error = "no such variable " + name;
		return nullptr;
	}

	JZNodeSetParam* node = createNode<JZNodeSetParam>();
	node->setVariable(info->name);
	return node;
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

bool JZScriptConvert::toReturn(asCScriptNode* node)
{
	Q_ASSERT(node->nodeType == snReturn);

	JZNodeReturn* ret = createNode<JZNodeReturn>();
	ret->setFunction(&m_script->function());
	if (node->firstChild)
	{
		JZNode* data = toAssignment(node->firstChild);
		if (!data)
			return false;

		if (m_script->function().paramOut.size() != 1)
		{
			m_error = "not return param";
			return false;
		}

		m_script->addConnectForce(data->paramOutGemo(0), ret->paramInGemo(0));
	}
	currentBlock()->flowList << ret;
	return true;
}

bool JZScriptConvert::setNodeIf(JZNodeIf* node_if, asCScriptNode* as_node, int cond)
{
	Q_ASSERT(as_node->nodeType == snIf);
	auto child = nodeChilds(as_node);

	pushBlock();
	if (node_if->subFlowCount() <= cond)
		node_if->addCondPin();

	auto jz_cond = toAssignment(child[0]);
	if (!jz_cond)
		return false;
	m_script->addConnectForce(jz_cond->paramOutGemo(0), node_if->paramInGemo(cond));

	QList<JZNode*> stmt;
	if (!toStatementBlock(child[1], stmt))
		return false;

	m_script->addConnect(node_if->subFlowOutGemo(cond), stmt[0]->flowInGemo());
	popBlock();

	if (child.size() > 2)
	{
		if (child[2]->nodeType == snIf)
			return setNodeIf(node_if, child[2], cond + 1);
		else
		{
			node_if->addElsePin();

			pushBlock();
			QList<JZNode*> else_stmt;
			if(!toStatementBlock(child[2], else_stmt))
				return false;
			popBlock();

			int else_index = node_if->subFlowCount() - 1;
			m_script->addConnect(node_if->subFlowOutGemo(else_index), else_stmt[0]->flowInGemo());
			return true;
		}
	}
	else
	{
		return true;
	}
}

bool JZScriptConvert::toIf(asCScriptNode* as_node)
{
	JZNodeIf* node_if = createNode<JZNodeIf>();
	if (!setNodeIf(node_if, as_node, 0))
		return false;

	currentBlock()->flowList << node_if;
	return node_if;
}

bool JZScriptConvert::toFor(asCScriptNode* node)
{
    Q_ASSERT(node->nodeType == snFor);

	JZScriptItemVisitor visitor(m_script);

	pushBlock();
    auto childs = nodeChilds(node);
	auto cur_block = currentBlock();

	//declartion
	JZNode* delcare = nullptr;
	if (childs[0]->nodeType == snDeclaration)
	{
		if (!toDeclarationStatement(childs[0]))
			return false;

		delcare = cur_block->flowList[0];
		cur_block->flowList.clear();
	}
	else
	{
		delcare = toExpressionStatementFlow(childs[0]);
		if (!delcare)
			return false;
	}

	QString var_name = delcare->paramInValue(0);
	auto init_list = visitor.dataInputNode(delcare, delcare->paramIn(1));
	if (init_list.size() != 1)
	{
		m_error = "no support init";
		return false;
	}

	m_script->removeNode(delcare->id());
	auto init = init_list[0];

	//cmp
	JZNode* cmp = toExpressionStatement(childs[1]);
	if (!cmp)
		return false;

	JZNodeIRType for_op = OP_none;
	if (cmp->type() == Node_lt)
		for_op = OP_lt;
	else if (cmp->type() == Node_le)
		for_op = OP_le;
	else if (cmp->type() == Node_gt)
		for_op = OP_gt;
	else if (cmp->type() == Node_ge)
		for_op = OP_ge;
	else if (cmp->type() == Node_eq)
		for_op = OP_eq;
	else if (cmp->type() == Node_ne)
		for_op = OP_ne;
	else
	{
		m_error = "not support";
		return false;
	}
	
	auto var_list = visitor.dataInputNode(cmp, cmp->paramIn(0));
	auto cond_list = visitor.dataInputNode(cmp, cmp->paramIn(1));
	if (var_list.size() != 1 || cond_list.size() != 1)
		return false;

	JZNode* var = var_list[0];
	JZNode* cond = cond_list[0];
	m_script->removeNode(var->id());
	m_script->removeNode(cmp->id());

	//next， 解析后应该是 i = i + step_cond 这样的格式，提取出 step_cond
	JZNode* next = toExpressionStatementFlow(childs[2]);
	auto next_list = visitor.dataInputNode(next, next->paramIn(1));
	Q_ASSERT(next_list.size() == 1);

	m_script->removeNode(next->id());
	auto step = next_list[0];
	JZNode* step_cond = nullptr;
	if (step->type() == Node_add || step->type() == Node_sub)
	{
		auto var_list = visitor.dataInputNode(step, step->paramIn(0));
		auto step_list = visitor.dataInputNode(step, step->paramIn(1));
		step_cond = step_list[0];
		m_script->removeNode(var_list[0]->id());
		m_script->removeNode(step->id());

		if (step->type() == Node_sub)
		{
			auto jz_not = createNode<JZNodeNot>();
			m_script->addConnect(step_cond->paramInGemo(0), jz_not->paramOutGemo(0));
			step_cond = jz_not;
		}
	}
	else
	{
		m_error = "only support +/-";
		return false;
	}
	
	//body
	JZNodeFor* node_for = createNode<JZNodeFor>();
	node_for->setOp(for_op);
	
	m_script->addConnect(init->paramOutGemo(0), node_for->paramInGemo(0));  //start
	m_script->addConnect(step_cond->paramOutGemo(0), node_for->paramInGemo(1));  //step
	m_script->addConnect(cond->paramOutGemo(0), node_for->paramInGemo(2));  //end

	QList<JZNode*> sub_list;
	if (!toStatementBlock(childs[3], sub_list))
		return false;

	JZNodeSetParam* set_param = createNode<JZNodeSetParam>();
	set_param->setVariable(var_name);
	m_script->addConnect(node_for->paramOutGemo(0), set_param->paramInGemo(1));

	m_script->addConnect(node_for->subFlowOutGemo(0), set_param->flowInGemo());
	m_script->addConnect(set_param->flowOutGemo(0), sub_list[0]->flowInGemo());
	
	popBlock();

	currentBlock()->flowList << node_for;
	return node_for;
}

JZNode *JZScriptConvert::toExpressionStatement(asCScriptNode* node)
{
	if (node->firstChild->nodeType == snAssignment)
	{
		return toAssignment(node->firstChild);
	}

	Q_ASSERT(0);
	return nullptr;
}

JZNode* JZScriptConvert::toExpressionStatementFlow(asCScriptNode* node)
{
	JZNode* jz_node = toExpressionStatement(node);
	if (!jz_node)
		return nullptr;

	auto cur_block = currentBlock();
	if (!jz_node->isFlowNode())
	{
		m_script->removeNode(jz_node->id());
		if (cur_block->preStatment)
		{
			jz_node = cur_block->preStatment;
			cur_block->preStatment = nullptr;
		}
		else if (cur_block->postStatment)
		{
			jz_node = cur_block->postStatment;
			cur_block->postStatment = nullptr;
		}
		else
		{
			m_error = "must give a expr";
			return false;
		}
	}

	return jz_node;
}

JZNode* JZScriptConvert::toAssignment(asCScriptNode* node)
{
	Q_ASSERT(node->nodeType == snAssignment);

	auto isContainerGet = [this](JZNode *node)->bool {
		if (node->type() != Node_function)
			return false;

		auto node_func = dynamic_cast<JZNodeFunction*>(node);
		auto func_def = this->function(node_func->function());
		return func_def && func_def->name == "get" && JZNodeType::isContainerType(func_def->className);
	};

	auto list = nodeChilds(node);
	if (list.size() == 1)
	{
		return toExpression(list[0]->firstChild);
	}
	else if (list.size() == 3)
	{
		if (list[1]->nodeType == snExprOperator)
		{
			QString op = nodeText(list[1]);			
			JZNode* node_get = toExpression(list[0]->firstChild);
            if (!node_get)
                return nullptr;
			
			JZNode* node_set = nullptr;
			int node_set_index = -1;
			if (node_get->type() == Node_param)
			{
				node_set = createSetParam(node_get->paramOutValue(0));
				if (!node_set)
					return nullptr;

				node_set_index = 1;
			}
			else if(isContainerGet(node_get))
			{
				auto get_func = dynamic_cast<JZNodeFunction*>(node_get);
				auto func_def = this->function(get_func->function());
				QString set_func_name = func_def->className + "::set";

				JZNodeFunction *set_func = createNode<JZNodeFunction>();
				set_func->setFunction(set_func_name);
				node_set = set_func;

				//self
				auto get_self_line_id = m_script->getConnectInput(get_func->id(), get_func->paramIn(0));
				auto get_self_line = m_script->getConnect(get_self_line_id[0]);
				m_script->addConnect(get_self_line->from, set_func->paramInGemo(0));

				//index
				auto get_index_line_id = m_script->getConnectInput(get_func->id(), get_func->paramIn(1));
				auto get_index_line = m_script->getConnect(get_index_line_id[0]);
				m_script->addConnect(get_index_line->from, set_func->paramInGemo(1));

				node_set_index = 2;
			}
			else
			{
				m_error = nodeText(list[0]) + " 不支持ֵ";
				return nullptr;
			}

            auto node_value = toAssignment(list[2]);
            if (op == "=")
            {
				m_script->removeNode(node_get->id());
                m_script->addConnectForce(node_value->paramOutGemo(0), node_set->paramInGemo(node_set_index));
            }
            else if(op.endsWith("="))
            {
                JZNode *op_node = createOpNode(op.left(1));
                
                m_script->addConnectForce(node_get->paramOutGemo(0), op_node->paramInGemo(0));
                m_script->addConnectForce(node_value->paramOutGemo(0), op_node->paramInGemo(1));
                m_script->addConnectForce(op_node->paramOutGemo(0), node_set->paramInGemo(node_set_index));
            }
			else
			{
				Q_ASSERT(0);
			}
			return node_set;
		}
	}
	
	Q_ASSERT(0);
	return nullptr;
}

JZNodeFunction* JZScriptConvert::createFunction(QString function_name, asCScriptNode* node)
{
	auto func_define = function(function_name);
	if (!func_define)
	{
		m_error = JZNodeCompiler::errorString(Error_noFunction, { function_name });
		return nullptr;
	}

	//check param
	auto node_name = node->firstChild;
	auto arg_list = nodeChilds(node_name->next);
	int allow_in = func_define->paramIn.size();
	int param_start = 0;
	if (func_define->isMemberFunction())
	{
		param_start++;
		allow_in--;
	}
	if (arg_list.size() != allow_in)
	{
		m_error = JZNodeCompiler::errorString(Error_functionParamIn, { QString::number(allow_in), QString::number(arg_list.size()) }); 
		return nullptr;
	}

	//create node
	JZNodeFunction* func = createNode<JZNodeFunction>();
	func->setFunction(func_define);
	for (int i = 0; i < arg_list.size(); i++)
	{
		auto param = toAssignment(arg_list[i]);
		m_script->addConnectForce(param->paramOutGemo(0), func->paramInGemo(i + param_start));
	}

	return func;
}

bool JZScriptConvert::toArgList(asCScriptNode* node, QList<JZNode*>& ret_list)
{
	Q_ASSERT(node->nodeType == snArgList);

	QList<JZNode*> jz_list;
	auto arg_list = nodeChilds(node);
	for (int i = 0; i < arg_list.size(); i++)
	{
		auto param = toAssignment(arg_list[i]);
		if (!param)
			return false;

		jz_list << param;
	}
	ret_list = jz_list;
	return true;
}

JZNode* JZScriptConvert::toFunctionCall(asCScriptNode* node)
{
	Q_ASSERT(node->nodeType == snFunctionCall);

	auto node_name = node->firstChild;
	QString func_name = nodeText(node_name);
	return createFunction(func_name, node);
}

JZNode* JZScriptConvert::toMemberFunctionCall(JZNode* self, asCScriptNode* node)
{
	QStringList pin_list = self->pinType(self->paramOut(0));
	Q_ASSERT(pin_list.size() == 1);

	QString class_name = pin_list[0];
	QString func_name = JZNodeType::baseType(class_name) + "::" + nodeText(node->firstChild);
	return createFunction(func_name, node);
}

bool JZScriptConvert::toFunctionCallStatement(asCScriptNode* node)
{
	JZNode* jz_node = toFunctionCall(node);
	if (!jz_node)
		return false;

	currentBlock()->flowList << jz_node;
	return true;
}

JZNode* JZScriptConvert::toExprTerm(asCScriptNode* root)
{
	Q_ASSERT(root->nodeType == snExprTerm);    

	auto env = environment();
	JZNode* ret = nullptr;
	auto node_value = root->firstChild;
	JZNode* pre_node = nullptr;
	asCScriptNode* pre = nullptr;
	if (node_value->nodeType == snExprPreOp) 
	{
		pre = node_value;
		if (pre->tokenType == ttDec || pre->tokenType == ttInc)
		{
			if (currentBlock()->preStatment)
			{
				m_error = "two pre in one statment";
				return nullptr;
			}

			JZNode* op = createOpNode(pre->tokenType == ttDec ? "-" : "+");

			auto param = createGetParam(nodeText(pre->next));
			auto set_param = createSetParam(nodeText(pre->next));
			auto literal = createNode<JZNodeLiteral>();
			literal->setDataType(Type_int);
			literal->setLiteral("1");

			m_script->addConnectForce(param->paramOutGemo(0), op->paramInGemo(0));
			m_script->addConnectForce(literal->paramOutGemo(0), op->paramInGemo(1));
			m_script->addConnectForce(op->paramOutGemo(0), set_param->paramInGemo(1));
			currentBlock()->preStatment = set_param;
		}
		else
		{
			pre_node = createSingleOpNode(nodeText(node_value));
		}
		node_value = node_value->next;
	}

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
		auto param = createGetParam(nodeText(node));
		ret = param;
	}
	else if (node->nodeType == snFunctionCall)
	{
		ret = toFunctionCall(node);
	} 
    else if (node->nodeType == snConstructCall)
    {
        auto list = nodeChilds(node);
        QString dataType = nodeText(list[0]);
        QString dataValue = nodeText(list[1]);
        qDebug() << dataType << dataValue;
		Q_ASSERT(0);
    }
	else
	{
		Q_ASSERT(0);
	}
	if (!ret)
		return nullptr;

	asCScriptNode* post = node_value->next;
	if (pre && post)
	{
		m_error = "pre post in one statment";
		return nullptr;
	}

	if (post && post->nodeType == snExprPostOp)
	{
		if (currentBlock()->preStatment)
		{
			m_error = "two post in one statment";
			return nullptr;
		}

		if (post->tokenType == ttDec || post->tokenType == ttInc)
		{
			JZNode *op = createOpNode(post->tokenType == ttDec? "-":"+");

			auto param = createGetParam(nodeText(node));
			auto set_param = createSetParam(nodeText(node));
			auto literal = createNode<JZNodeLiteral>();
			literal->setDataType(Type_int);
			literal->setLiteral("1");

			m_script->addConnectForce(param->paramOutGemo(0), op->paramInGemo(0));
			m_script->addConnectForce(literal->paramOutGemo(0), op->paramInGemo(1));
			m_script->addConnectForce(op->paramOutGemo(0), set_param->paramInGemo(1));
			currentBlock()->postStatment = set_param;
		}
		else if (post->tokenType == ttDot)
		{
			JZNode* node_call = toMemberFunctionCall(ret, post->firstChild);
			if (!node_call)
				return nullptr;

			m_script->addConnect(ret->paramOutGemo(0), node_call->paramInGemo(0));
			ret = node_call;
		}
		else if (post->tokenType == ttOpenBracket)
		{
			QString param_name = nodeText(node);
			auto var_def = getVariableInfo(param_name);
			if (!JZNodeType::isContainerType(var_def->type))
			{
				m_error = param_name + "is node container";
				return nullptr;
			}
			JZNodeFunction* func = createNode<JZNodeFunction>();
			func->setFunction(JZNodeType::baseType(var_def->type) + "::get");

			JZNode *assign = toAssignment(post->firstChild->firstChild);
			if (!assign)
				return nullptr;

			m_script->addConnect(ret->paramOutGemo(0), func->paramInGemo(0));
			m_script->addConnect(assign->paramOutGemo(0), func->paramInGemo(1));
			ret = func;
		}
		else
		{
			m_error = "un support post op " + nodeText(post);
			return nullptr;
		}
	}
	else if (post)
	{
		Q_ASSERT(0);
	}

	if (pre_node)
	{
		m_script->addConnect(ret->paramOutGemo(0), pre_node->paramInGemo(0));
		ret = pre_node;
	}

	return ret;
}

bool JZScriptConvert::toWhile(asCScriptNode* node)
{    
    auto list = nodeChilds(node);

	pushBlock();
	JZNodeWhile* node_while = createNode<JZNodeWhile>();
    auto expr = toAssignment(list[0]);
	if (!expr)
		return false;

	QList<JZNode*> body;
	if (!toStatementBlock(list[1], body))
		return false;
    
	m_script->addConnect(expr->paramOutGemo(0), node_while->paramInGemo(0));
    m_script->addConnect(node_while->subFlowOutGemo(0), body[0]->flowInGemo());
	popBlock();

	currentBlock()->flowList << node_while;
	return true;
}

bool JZScriptConvert::toSwitch(asCScriptNode* node)
{
	auto childs = nodeChilds(node);
	
	JZNodeSwitch* node_switch = createNode<JZNodeSwitch>();
	node_switch->clearCaseAndDefault();

	JZNode *cond = toAssignment(childs[0]);
	m_script->addConnect(cond->paramOutGemo(0), node_switch->paramInGemo(0));

	for(int i = 1 ; i < childs.size(); i++)
	{
		pushBlock();
		if (childs[i]->firstChild->nodeType == snExpression)
		{
			QString value = nodeText(childs[i]->firstChild);
			int flow_id = node_switch->addCase();
			node_switch->setPinValue(flow_id, value);
			
			QList<JZNode*> list;
			if (!toStatementBlock(childs[i]->firstChild->next, list))
				return false;

			m_script->addConnect(JZNodeGemo(node_switch->id(), flow_id), list[0]->flowInGemo());
		}
		else
		{
			int flow_id = node_switch->addDefault();
			
			QList<JZNode*> list;
			if (!toStatementBlock(childs[i]->firstChild, list))
				return false;

			m_script->addConnect(JZNodeGemo(node_switch->id(), flow_id), list[0]->flowInGemo());
		}
		popBlock();
	}

	currentBlock()->flowList << node_switch;
	return true;
}

bool JZScriptConvert::toBreak(asCScriptNode* node)
{
	JZNodeBreak* node_break = createNode<JZNodeBreak>();
	currentBlock()->flowList << node_break;
	return node_break;
}

bool JZScriptConvert::toContinue(asCScriptNode* node)
{
	JZNodeContinue* node_continue = createNode<JZNodeContinue>();
	currentBlock()->flowList << node_continue;
	return node_continue;
}

bool JZScriptConvert::toStatementBlock(asCScriptNode* node, QList<JZNode*> &list)
{
	if (node->nodeType != snStatementBlock)
	{
		if (!toStatement(node))
			return false;

		list = currentBlock()->flowList;
		return true;
	}

	JZNode* pre = nullptr;
	auto child = node->firstChild;
	while (child)
	{        
		if (!toStatement(child))
			return false;

		auto cur_block = currentBlock();
		//pre
		if (cur_block->preStatment)
		{
			list.push_back(cur_block->preStatment);
			if(pre)
				m_script->addConnect(pre->flowOutGemo(), cur_block->preStatment->flowInGemo());
			pre = cur_block->preStatment;
			cur_block->preStatment = nullptr;
		}

		//flow
		if (cur_block->flowList.size() > 0)
		{
			if (pre)
				m_script->addConnect(pre->flowOutGemo(), cur_block->flowList.front()->flowInGemo());

			list.append(cur_block->flowList);
			pre = cur_block->flowList.back();
			cur_block->flowList.clear();
		}

		//post
		if (cur_block->postStatment)
		{
			list.push_back(cur_block->postStatment);
			m_script->addConnect(pre->flowOutGemo(), cur_block->postStatment->flowInGemo());
			pre = cur_block->postStatment;
			cur_block->postStatment = nullptr;
		}
		
		child = nextNode(child, 1);
	}
	return true;
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

	auto list = nodeChilds(node);
	auto opPri = [](const ExprToken& tk)->int {
		auto* op_node = dynamic_cast<const JZNodeDoubleOperator*>(tk.node);
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
			if (!tk.node)
				return nullptr;

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

bool JZScriptConvert::toDeclarationStatement(asCScriptNode* node)
{
	Q_ASSERT(node->nodeType == snDeclaration);

	auto list = nodeChilds(node);
	
	QString data_type = toDataType(list[0]);
	QString name = nodeText(list[1]);
	
	if (getVariableInfo(name))
	{
		m_error = JZNodeCompiler::errorString(Error_varAllreadyDefined, { name });
		return false;
	}
	addLocalVariable(name, data_type);

	JZNode* expr = nullptr;
	if (list.size() == 2)
	{

	}
	else
	{
		if (list[2]->nodeType == snArgList)
		{
			QList<JZNode*> arg_list;
			if (!toArgList(list[2], arg_list))
				return false;

			//构造函数
			expr = createObject(data_type);
		}
		else
		{
			expr = toAssignment(list[2]);
			if (!expr)
				return false;

			Q_ASSERT(!expr->isFlowNode());
		}

		JZNodeSetParam* set = createSetParam(name);
		m_script->addConnectForce(expr->paramOutGemo(0), set->paramInGemo(1));
		currentBlock()->flowList << set;
	}

	return true;
}

JZScriptConvert::BlockEnv* JZScriptConvert::currentBlock()
{
	return m_blockEnv.back().data();
}

void JZScriptConvert::pushBlock()
{
	BlockEnvPtr block = BlockEnvPtr(new BlockEnv());
	m_blockEnv.push_back(block);
}

void JZScriptConvert::popBlock()
{
	m_blockEnv.pop_back();
}