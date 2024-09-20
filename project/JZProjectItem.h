#ifndef JZPROJECT_ITEM_FILE_H_
#define JZPROJECT_ITEM_FILE_H_
#include <QSharedPointer>

enum {
    ProjectItem_any = -1,
    ProjectItem_none,
    ProjectItem_root,
    ProjectItem_folder,    
    ProjectItem_ui,
    ProjectItem_param,
    ProjectItem_class,
    ProjectItem_scriptFile,
    ProjectItem_scriptParamBinding,
    ProjectItem_scriptFunction,    
    ProjectItem_library,
};

class JZProject;
class JZScriptClassItem;
class JZEditor;
class JZProjectItem
{    
public:
    JZProjectItem(int itemType);
    virtual ~JZProjectItem();

    void setProject(JZProject *project);
    JZProject *project();

    void setEditor(JZEditor *editor);
    JZEditor *editor();  

    QString name() const;
    void setName(QString name);    

    QString path();
    QString itemPath();

    int itemType() const;
    void setItemType(int type);

    JZProjectItem *parent();
    JZScriptClassItem *getClassFile();
    
    JZProjectItem *getItem(QString name);     
    bool hasItem(QString name);
    int indexOfItem(JZProjectItem *item);
    QList<JZProjectItem *> childs();
    int childCount();

    void sort();
    QList<JZProjectItem *> itemList(int type);
    QList<JZProjectItem *> itemList(QList<int> type);
    
protected:
    friend JZProject;
    
    Q_DISABLE_COPY(JZProjectItem)

    void addItem(QSharedPointer<JZProjectItem> child);
    void removeItem(int index);    
    void removeChlids();    

    JZProjectItem *m_parent;
    QList<QSharedPointer<JZProjectItem>> m_childs;
    
    int m_itemType;    
    QString m_name;    
    JZProject *m_project;
    JZEditor *m_editor;
    int m_pri;
};
typedef QSharedPointer<JZProjectItem> JZProjectItemPtr;

//JZProjectItemRoot
class JZProjectItemRoot : public JZProjectItem
{
public:
    JZProjectItemRoot();
    virtual ~JZProjectItemRoot();
};

//JZProjectItemFolder
class JZProjectItemFolder : public JZProjectItem
{
public:
    JZProjectItemFolder();
    virtual ~JZProjectItemFolder();
};

int JZProjectItemIsScript(JZProjectItem *item);

#endif
