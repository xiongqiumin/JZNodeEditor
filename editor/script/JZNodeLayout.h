#ifndef JZNODE_LAYOUT_H_
#define JZNODE_LAYOUT_H_

#include <QList>
#include <QMap>
#include <QSet>
#include "JZNodeCompiler.h"

class JZNodeLayoutNode
{
public:
    JZNodeLayoutNode();
    void clear();

    int nodeId;
    int row, col;    

    QList<int> input;
    QList<int> output;
};
typedef QSharedPointer<JZNodeLayoutNode> LayoutNodePtr;

class JZNodeView;
class JZNodeLayoutTree
{
public:
    JZNodeLayoutTree();

    void make(Graph *graph);
    JZNodeLayoutNode *getNode(int id);
    int calcNodeOuput(int id,int row);
    int calcNodeInput(int id,int row,int &user_row);
    int toInt(int row, int col);
    
    QMap<int, LayoutNodePtr> m_nodeMap;
    int gap;
    JZNodeView *m_view;
    Graph *m_graph;
    QSet<int> m_posCache;
    QMap<int,JZNodeLayoutNode*> m_nodeCache;
};




#endif // !JZNODE_LAYOUT_H_
