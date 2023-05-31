#ifndef JZPROJECT_ITEM_FILE_H_
#define JZPROJECT_ITEM_FILE_H_
#include <QSharedPointer>

enum{    
    ProjectItem_root,
    ProjectItem_folder,
    ProjectItem_ui,
    ProjectItem_param,
    ProjectItem_scriptParam,
    ProjectItem_scriptFlow,    
    ProjectItem_scriptFunction,
};

class JZProject;
class JZProjectItem
{    
public:
    JZProjectItem(int itemType);
    virtual ~JZProjectItem();

    void setProject(JZProject *project);
    JZProject *project() const;

    QString name();
    void setName(QString name);

    QString path();
    QString itemPath();

    int itemType();
    void setItemType(int type);

    bool isFolder();
    JZProjectItem *parent();
    
    void addItem(QSharedPointer<JZProjectItem> child);
    void removeItem(int index);
    JZProjectItem *getItem(QString name);     
    int indexOfItem(JZProjectItem *);   
    QList<JZProjectItem *> childs();
    void removeChlids();

    virtual void saveToStream(QDataStream &s);
    virtual void loadFromStream(QDataStream &s);

    void sort();
    
protected:
    Q_DISABLE_COPY(JZProjectItem)

    JZProjectItem *m_parent;
    QList<QSharedPointer<JZProjectItem>> m_childs;
    
    int m_itemType;    
    QString m_name;    
    bool m_folder;
    JZProject *m_project;
};
typedef QSharedPointer<JZProjectItem> JZProjectItemPtr;

//JZProjectItemFolder
class JZProjectItemFolder : public JZProjectItem
{
public:
    JZProjectItemFolder();
    virtual ~JZProjectItemFolder();
};

//JZProjectItemFactory
class JZProjectItemFactory
{
public:
    static JZProjectItem *create(int itemType);
    static QString itemTypeName(int itemType);
};

#endif
