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
    ProjectItem_scriptFlow,
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
    JZProject *project() const;

    void setEditor(JZEditor *editor);
    JZEditor *editor() const;  

    QString name();
    void setName(QString name);    

    QString path();
    QString itemPath();

    int itemType();
    void setItemType(int type);

    JZProjectItem *parent();
    JZScriptClassItem *getClassFile();
    QString className();
    
    void addItem(QSharedPointer<JZProjectItem> child);
    void removeItem(int index);    
    JZProjectItem *getItem(QString name);     
    bool hasItem(QString name);
    int indexOfItem(JZProjectItem *item);
    QList<JZProjectItem *> childs();
    void removeChlids();

    void sort();
    QList<JZProjectItem *> itemList(int type);
    QList<JZProjectItem *> itemList(QList<int> type);
    
protected:
    Q_DISABLE_COPY(JZProjectItem)

    void regist();

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
