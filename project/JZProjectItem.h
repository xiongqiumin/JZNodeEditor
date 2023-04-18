#ifndef JZPROJECT_ITEM_FILE_H_
#define JZPROJECT_ITEM_FILE_H_
#include <QSharedPointer>

enum{    
    ProjectItem_root,
    ProjectItem_ui,
    ProjectItem_param,
    ProjectItem_scriptParam,
    ProjectItem_scriptFlow,
    ProjectItem_scriptFunction,
};

class JZProjectItem
{    
public:
    JZProjectItem(int itemType,bool folder);
    virtual ~JZProjectItem();

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
    int indexOfItem(JZProjectItem *);

    virtual void saveToStream(QDataStream &s);
    virtual void loadFromStream(QDataStream &s);

    void sort();
    
protected:        
    JZProjectItem *m_parent;
    QList<JZProjectItem*> m_childs;
    
    int m_itemType;
    int m_subType;
    QString m_name;    
    bool m_folder;
};
typedef QSharedPointer<JZProjectItem> JZProjectItemPtr;

//JZProjectRoot
class JZProjectRoot : public JZProjectItem
{
public:
    JZProjectRoot();
    virtual ~JZProjectRoot();
};

//JZProjectItemFactory
class JZProjectItemFactory
{
public:
    static JZProjectItem *create(int itemType,bool folder);
    static QString itemTypeName(int itemType);
};

#endif
