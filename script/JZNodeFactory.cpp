#include "JZNodeFactory.h"
#include "JZNodeValue.h"
#include "JZNodeExpression.h"
#include "JZNodeEvent.h"
#include "JZNodeFlow.h"
#include "JZNodeFunction.h"
#include "JZNodeOperator.h"

JZNodeFactory::JZNodeFactory()
{   
    registNode(Node_print,createJZNode<JZNodePrint>);  
    registNode(Node_format,createJZNode<JZNodeFormat>);  
    registNode(Node_formatBin, createJZNode<JZNodeFormatBin>);
    registNode(Node_nop, createJZNode<JZNodeNop>);
    registNode(Node_assert, createJZNode<JZNodeAssert>);
    registNode(Node_display, createJZNode<JZNodeDisplay>);

    registNode(Node_literal,createJZNode<JZNodeLiteral>);    
    registNode(Node_enum,createJZNode<JZNodeEnum>);
    registNode(Node_flag,createJZNode<JZNodeFlag>);
    registNode(Node_create,createJZNode<JZNodeCreateObject>);
    registNode(Node_createFromString, createJZNode<JZNodeCreateFromString>);
    registNode(Node_this,createJZNode<JZNodeThis>);
    registNode(Node_param,createJZNode<JZNodeParam>);
    registNode(Node_setParam,createJZNode<JZNodeSetParam>);
    registNode(Node_memberParam, createJZNode<JZNodeMemberParam>);
    registNode(Node_setMemberParam, createJZNode<JZNodeSetMemberParam>);
    registNode(Node_clone, createJZNode<JZNodeClone>);
    registNode(Node_swap, createJZNode<JZNodeSwap>);    
    registNode(Node_convert, createJZNode<JZNodeConvert>);    
    
    registNode(Node_functionStart, createJZNode<JZNodeFunctionStart>);
    registNode(Node_function,createJZNode<JZNodeFunction>);
    registNode(Node_while,createJZNode<JZNodeWhile>);
    registNode(Node_for,createJZNode<JZNodeFor>);
    registNode(Node_foreach,createJZNode<JZNodeForEach>);
    registNode(Node_sequence,createJZNode<JZNodeSequence>);
    registNode(Node_tryCatch,createJZNode<JZNodeTryCatch>);
    registNode(Node_throw, createJZNode<JZNodeThrow>);
    registNode(Node_break,createJZNode<JZNodeBreak>);
    registNode(Node_continue,createJZNode<JZNodeContinue>);
    registNode(Node_return,createJZNode<JZNodeReturn>);
    registNode(Node_switch,createJZNode<JZNodeSwitch>);
    registNode(Node_if,createJZNode<JZNodeIf>);

    registNode(Node_add,createJZNode<JZNodeAdd>);
    registNode(Node_sub,createJZNode<JZNodeSub>);
    registNode(Node_mul,createJZNode<JZNodeMul>);
    registNode(Node_div,createJZNode<JZNodeDiv>);
    registNode(Node_mod,createJZNode<JZNodeMod>);
    registNode(Node_eq,createJZNode<JZNodeEQ>);
    registNode(Node_ne,createJZNode<JZNodeNE>);
    registNode(Node_le,createJZNode<JZNodeLE>);
    registNode(Node_ge,createJZNode<JZNodeGE>);
    registNode(Node_lt,createJZNode<JZNodeLT>);
    registNode(Node_gt,createJZNode<JZNodeGT>);
    registNode(Node_bitand, createJZNode<JZNodeBitAnd>);
    registNode(Node_bitor, createJZNode<JZNodeBitOr>);
    registNode(Node_bitxor, createJZNode<JZNodeBitXor>);
    registNode(Node_bitreverse, createJZNode<JZNodeBitReverse>);
    registNode(Node_and,createJZNode<JZNodeAnd>);
    registNode(Node_or,createJZNode<JZNodeOr>);
    registNode(Node_not, createJZNode<JZNodeNot>);
    registNode(Node_neg, createJZNode<JZNodeNeg>);
    
    registNode(Node_expr,createJZNode<JZNodeExpression>);

    registNode(Node_functionPointer, createJZNode<JZNodeFunctionPointer>);
    registNode(Node_signalConnect, createJZNode<JZNodeSignalConnect>);

    registNode(Node_mainLoop, createJZNode<JZNodeMainLoop>);

    registNode(Node_showEvent, createJZNode<JZNodeShowEvent>);
    registNode(Node_closeEvent, createJZNode<JZNodeCloseEvent>);
    registNode(Node_resizeEvent, createJZNode<JZNodeResizeEvent>);
    registNode(Node_paintEvent, createJZNode<JZNodePaintEvent>);

    registNode(Node_mouseMoveEvent, createJZNode<JZNodeMouseMoveEvent>);
    registNode(Node_mousePressEvent, createJZNode<JZNodeMousePressEvent>);
    registNode(Node_mouseReleaseEvent, createJZNode<JZNodeMouseReleaseEvent>);

    registNode(Node_keyPressEvent, createJZNode<JZNodeKeyPressEvent>);
    registNode(Node_keyReleaseEvent, createJZNode<JZNodeKeyReleaseEvent>);
    
    registNode(Node_buttonClickedEvent, createJZNode<JZNodeButtonClickedEvent>);
    registNode(Node_timerEvent, createJZNode<JZNodeTimerEvent>);
}

JZNodeFactory::~JZNodeFactory()
{

}

void JZNodeFactory::registNode(int type,JZNodeCreateFunc func)
{
    Q_ASSERT(type != Node_none && !m_nodes.count(type));
    m_nodes[type] = func;
}

QList<int> JZNodeFactory::nodeTypeList() const
{
    QList<int> types;
    auto it = m_nodes.begin();
    while(it != m_nodes.end())
    {
        types << it.key();
        it++;
    }
    return types;
}

JZNode *JZNodeFactory::createNode(int type) const
{
    auto it = m_nodes.find(type);
    Q_ASSERT(it != m_nodes.end());
    return it.value()();
}

JZNode *JZNodeFactory::loadNode(const QByteArray &buffer) const
{
    QDataStream node_s(buffer);
    int node_type;
    node_s >> node_type;

    JZNode *node = createNode(node_type);
    node->fromBuffer(buffer);
    return node;
}

QByteArray JZNodeFactory::saveNode(JZNode *node) const
{
    return node->toBuffer();
}