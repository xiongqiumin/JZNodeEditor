#ifndef JZPROJECT_ITEM_FILE_H_
#define JZPROJECT_ITEM_FILE_H_
#include <QSharedPointer>

enum{
    ProjectItem_none,
    ProjectItem_root,
};

class JZProjectItem
{    
public:
    JZProjectItem(int itemType,bool folder);
    ~JZProjectItem();

    QString name();
    void setName(QString name);

    QString path();
    QString itemPath();

    int itemType();
    void setItemType(int type);

    bool isFolder();
    JZProjectItem *parent();
    
    void addItem(JZProjectItem *child);
    void removeItem(JZProjectItem *child);
    JZProjectItem *getItem(QString name);
    QList<JZProjectItem *> items();

protected:        
    JZProjectItem *m_parent;
    QList<JZProjectItem*> m_childs;
    
    int m_itemType;
    QString m_name;    
    bool m_folder;
};
typedef QSharedPointer<JZProjectItem> JZProjectItemPtr;

#endif