#ifndef JZNODE_FACTORY_H_
#define JZNODE_FACTORY_H_

#include <QMap>
#include "JZNode.h"

typedef bool(*JZNodeFactoryEdit)(JZNode *node);

typedef JZNode *(*JZNodeCreateFunc)();
class JZNodeFactory
{
public:
    static JZNodeFactory *instance();

    void init();

    JZNode *createNode(int type);    
    void registNode(int type,JZNodeCreateFunc func);
    QList<int> nodeTypeList();

    bool edit(JZNode *node);
    void registEdit(int type,JZNodeFactoryEdit func);

protected:
    QMap<int,JZNodeCreateFunc> m_nodes;
    QMap<int,JZNodeFactoryEdit> m_edits;
};

//这两个函数保存了类型
JZNode *loadNode(const QByteArray &buffer);
QByteArray saveNode(JZNode *node);  
#endif
