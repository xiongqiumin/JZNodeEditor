﻿#ifndef JZNODE_FACTORY_H_
#define JZNODE_FACTORY_H_

#include <QMap>
#include "JZNode.h"

typedef bool(*JZNodeFactoryEdit)(JZNode *node);

typedef JZNode *(*JZNodeCreateFunc)();
class JZCORE_EXPORT JZNodeFactory
{
public:
    static JZNodeFactory *instance();

    void init();

    void registNode(int type, JZNodeCreateFunc func);
    QList<int> nodeTypeList();

    JZNode *createNode(int type);        
    JZNode *loadNode(const QByteArray &buffer);
    QByteArray saveNode(JZNode *node);

protected:
    QMap<int,JZNodeCreateFunc> m_nodes;
    QMap<int,JZNodeFactoryEdit> m_edits;
};

template<class T>
JZNode *createJZNode(){ return new T(); }

#endif
