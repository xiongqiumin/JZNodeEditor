#include "JZNodeFactory.h"

template<class T> 
JZNode *createFunc(){ return new T();}

JZNodeFactory *JZNodeFactory::instance()
{
    static JZNodeFactory inst;
    return &inst;
}

void JZNodeFactory::registNode(int type,JZNodeCreateFunc func)
{
    m_nodes[type] = func;
}

QList<int> JZNodeFactory::nodeTypeList()
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

JZNode *JZNodeFactory::createNode(int type)
{
    auto it = m_nodes.find(type);
    Q_ASSERT(it != m_nodes.end());
    return it.value()();
}

void JZNodeFactory::init()
{
    registNode(Node_event,createFunc<JZNodeEvent>);  
    registNode(Node_print,createFunc<JZNodePrint>);  
    registNode(Node_literal,createFunc<JZNodeLiteral>);    
    registNode(Node_param,createFunc<JZNodeParam>);
    registNode(Node_setParam,createFunc<JZNodeSetParam>);
    registNode(Node_setParamData,createFunc<JZNodeSetParamData>);
    
    registNode(Node_function,createFunc<JZNodeFunction>);
    registNode(Node_while,createFunc<JZNodeWhile>);
    registNode(Node_for,createFunc<JZNodeFor>);
    registNode(Node_branch,createFunc<JZNodeBranch>);
    //registNode(Node_switch,createFunc<JZNodeSwitch>);
    //registNode(Node_if,createFunc<JZNodeIf>);

    registNode(Node_add,createFunc<JZNodeAdd>);
    registNode(Node_sub,createFunc<JZNodeSub>);
    registNode(Node_mul,createFunc<JZNodeMul>);
    registNode(Node_div,createFunc<JZNodeDiv>);
    registNode(Node_mod,createFunc<JZNodeMod>);
    registNode(Node_eq,createFunc<JZNodeEQ>);
    registNode(Node_ne,createFunc<JZNodeNE>);
    registNode(Node_le,createFunc<JZNodeLE>);
    registNode(Node_ge,createFunc<JZNodeGE>);
    registNode(Node_lt,createFunc<JZNodeLT>);
    registNode(Node_gt,createFunc<JZNodeGT>);
    registNode(Node_and,createFunc<JZNodeAnd>);
    registNode(Node_or,createFunc<JZNodeOr>);
    registNode(Node_bitand,createFunc<JZNodeBitAnd>);
    registNode(Node_bitor,createFunc<JZNodeBitOr>);
    registNode(Node_bitxor,createFunc<JZNodeBitXor>);
    registNode(Node_expr,createFunc<JZNodeExpression>);
}

//parseNode
JZNode *parseNode(const QByteArray &buffer)
{
    QDataStream s(buffer);
    int type;
    s >> type;
    JZNode *node = JZNodeFactory::instance()->createNode(type);
    node->loadFromStream(s);
    return node;
}

QByteArray formatNode(JZNode *node)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << node->type();
    node->saveToStream(s);
    return data;
}
