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

    bool open(QString filepath);
    bool save();    
    QString filename();
    
    int addItem(QString dir,JZProjectItem *item);
    void removeItem(QString path);
    int renameItem(JZProjectItem *item,QString newname);
    JZProjectItem *getItem(QString path);
    QList<JZProjectItem*> items();
    JZProjectItem *root();

protected:
    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);
    void sort();
    void makeTree();    
    void clear();
        
    QList<JZProjectItemPtr> m_items;
    JZProjectRoot m_root;
    QString m_filepath;
};

#endif
