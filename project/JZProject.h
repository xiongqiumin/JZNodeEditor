#ifndef JZNODE_PROJECT_H_
#define JZNODE_PROJECT_H_

#include "JZNode.h"
#include <QMap>
#include "JZProjectItem.h"

class JZProject
{
public:
    JZProject();
    ~JZProject();

    void clear();
    void addFile(QString dir,JZProjectItem *item);
    void removeFile(QString path);        
    JZProjectItem *root();

protected:
    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);
    void sort();
    void makeTree();
    JZProjectItem *getItem(QString path);

    int m_nodeId;
    QMap<int, JZNodePtr> m_nodes;
    QList<JZNodeConnect> m_connects;
    QList<JZProjectItemPtr> m_files;
    JZProjectItem m_root;
};

#endif
