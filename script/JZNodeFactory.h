#ifndef JZNODE_FACTORY_H_
#define JZNODE_FACTORY_H_

#include <QMap>
#include "JZNode.h"

typedef JZNode *(*JZNodeCreateFunc)();
class JZNodeFactory
{
public:
    JZNodeFactory();
    ~JZNodeFactory();

    void init();
    void registNode(int type, JZNodeCreateFunc func);
    QList<int> nodeTypeList() const;

    JZNode *createNode(int type) const;
    JZNode *loadNode(const QByteArray &buffer) const;
    QByteArray saveNode(JZNode *node) const;

protected:
    QMap<int,JZNodeCreateFunc> m_nodes;
};

template<class T>
JZNode *createJZNode(){ return new T(); }

#endif
