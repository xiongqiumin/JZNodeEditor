#include "JZVisionStringTable.h"
#include "JZNodeFunction.h"
#include "JZScriptEnvironment.h"
#include "JZScriptItem.h"
#include "JZClassItem.h"

JZVisionStringTable* JZVisionStringTable::instance()
{
	static JZVisionStringTable inst;
	return &inst;
}

JZVisionStringTable::JZVisionStringTable()
{
    // 基础节点
    m_nodeTypeMap[Node_param] = "参数";
    m_nodeTypeMap[Node_create] = "创建";
    m_nodeTypeMap[Node_createFromString] = "从字符串创建";
    m_nodeTypeMap[Node_this] = "this";
    m_nodeTypeMap[Node_setParam] = "设置参数";
    m_nodeTypeMap[Node_memberParam] = "成员参数";
    m_nodeTypeMap[Node_setMemberParam] = "设置成员参数";
    m_nodeTypeMap[Node_literal] = "字面量";
    m_nodeTypeMap[Node_enum] = "枚举";
    m_nodeTypeMap[Node_flag] = "标志";
    m_nodeTypeMap[Node_convert] = "转换";
    m_nodeTypeMap[Node_functionPointer] = "函数指针";
    m_nodeTypeMap[Node_functionStart] = "函数开始";
    m_nodeTypeMap[Node_clone] = "克隆";
    m_nodeTypeMap[Node_assert] = "断言";
    m_nodeTypeMap[Node_swap] = "交换";

    // 运算符节点
    m_nodeTypeMap[Node_add] = "加法";
    m_nodeTypeMap[Node_sub] = "减法";
    m_nodeTypeMap[Node_mul] = "乘法";
    m_nodeTypeMap[Node_div] = "除法";
    m_nodeTypeMap[Node_mod] = "取模";
    m_nodeTypeMap[Node_eq] = "等于";
    m_nodeTypeMap[Node_ne] = "不等于";
    m_nodeTypeMap[Node_le] = "小于等于";
    m_nodeTypeMap[Node_ge] = "大于等于";
    m_nodeTypeMap[Node_lt] = "小于";
    m_nodeTypeMap[Node_gt] = "大于";
    m_nodeTypeMap[Node_bitand] = "按位与";
    m_nodeTypeMap[Node_bitor] = "按位或";
    m_nodeTypeMap[Node_bitxor] = "按位异或";
    m_nodeTypeMap[Node_and] = "逻辑与";
    m_nodeTypeMap[Node_or] = "逻辑或";
    m_nodeTypeMap[Node_bitreverse] = "按位取反";
    m_nodeTypeMap[Node_not] = "逻辑非";
    m_nodeTypeMap[Node_neg] = "负号";
    m_nodeTypeMap[Node_expr] = "表达式";

    // 控制流节点
    m_nodeTypeMap[Node_for] = "for";
    m_nodeTypeMap[Node_foreach] = "foreach";
    m_nodeTypeMap[Node_while] = "while";
    m_nodeTypeMap[Node_sequence] = "序列";
    m_nodeTypeMap[Node_if] = "条件判断";
    m_nodeTypeMap[Node_parallel] = "并行";

    m_nodeTypeMap[Node_format] = "字符串格式化";
    m_nodeTypeMap[Node_formatBin] = "二进制格式化";
    m_nodeTypeMap[Node_print] = "打印";
    m_nodeTypeMap[Node_log] = "日志";
    m_nodeTypeMap[Node_display] = "显示";
    
    m_nodeTypeMap[Node_switch] = "选择";
    m_nodeTypeMap[Node_break] = "跳出循环";
    m_nodeTypeMap[Node_continue] = "继续循环";
    m_nodeTypeMap[Node_return] = "返回";
    m_nodeTypeMap[Node_tryCatch] = "异常捕获";
    m_nodeTypeMap[Node_throw] = "抛出异常";

    // 其他节点
    m_nodeTypeMap[Node_signalConnect] = "信号连接";
    m_nodeTypeMap[Node_mainLoop] = "主循环";
}

JZVisionStringTable::~JZVisionStringTable()
{
}

QString JZVisionStringTable::functionName(JZNode * node,  QString current_class)
{
    QString functionName;
    if (node->type() == Node_function)
    {
        auto node_func = dynamic_cast<JZNodeFunction*>(node);
        if (node_func->isDirectCall())
            return node_func->function();

        functionName = node_func->function();
    }
    else
    {
        auto node_func = dynamic_cast<JZNodeContainerFunction*>(node);
        functionName = node_func->function();
    }

    auto func = node->environment()->function(functionName);
    if (!func)
        return functionName;

    if (!func->isMemberFunction())
        return func->fullName();

    QString variable = node->paramInValue(0);
    auto this_id = node->paramIn(0);
    if (node->file()->getConnectInput(node->id(), this_id).size() > 0)
        return functionName;

    if (current_class == func->className)
    {
        if (variable.isEmpty() || variable == "this")
            return func->name;
    }

    if (variable.isEmpty())
        return func->fullName();
    else
        return variable + "." + func->name;
}

QString JZVisionStringTable::nodeName(JZNode* node)
{
    auto env = node->environment();
    auto script = node->file();
    QString class_name;
    if (script->getClassItem())
        class_name = script->getClassItem()->className();

	QString name = m_nodeTypeMap.value(node->type());
    if(!name.isEmpty())
        return name;

    if(node->type() == Node_function || node->type() == Node_genericFunction)
        return functionName(node, class_name);

    return node->name();
}