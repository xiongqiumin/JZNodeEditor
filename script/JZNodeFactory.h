#ifndef JZNODE_FACTORY_H_
#define JZNODE_FACTORY_H_

#include <QMap>
#include "JZNode.h"
#include "JZNodeValue.h"
#include "JZNodeExpression.h"
#include "JZNodeEvent.h"
#include "JZNodeFunction.h"

typedef JZNode *(*JZNodeCreateFunc)();
class JZNodeFactory
{
public:
    static JZNodeFactory *instance();

    void init();

    JZNode *createNode(int type);    
    void registNode(int type,JZNodeCreateFunc func);
    QList<int> nodeTypeList();

protected:
    QMap<int,JZNodeCreateFunc> m_nodes;
};

//这两个函数保存了类型
JZNode *loadNode(const QByteArray &buffer);
QByteArray saveNode(JZNode *node);  
#endif
