#ifndef JZPROJECT_ITEM_FILE_H_
#define JZPROJECT_ITEM_FILE_H_

#include <QMap>
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
    ProjectItem_scriptFunction,
};

class JZProject;
class JZScriptClassItem;
class JZEditor;
class JZProjectItem
{    
public:
    JZProjectItem(int itemType);
    virtual ~JZProjectItem();
    
    JZProject *project();
    const JZProject *project() const;

    QByteArray toBuffer() const;
    void fromBuffer(const QByteArray &buffer);

    QString name() const;
    void setName(QString name);    

    QString path();
    QString itemPath();

    int itemType() const;
    void setItemType(int type);

    JZProjectItem *parent();
    JZScriptClassItem *getClassItem();
    
    JZProjectItem *getItem(QString name);     
    bool hasItem(QString name);
    int indexOfItem(JZProjectItem *item);
    QList<JZProjectItem *> childs();
    int childCount();

    QList<JZProjectItem *> itemList(int type);
    QList<JZProjectItem *> itemList(QList<int> type);
    
protected:
    friend JZProject;
    
    Q_DISABLE_COPY(JZProjectItem)

    void addItem(JZProjectItem *child);
    void removeItem(JZProjectItem* child);
    void clearChlids();    

    virtual void saveToStream(QDataStream &s) const;
    virtual bool loadFromStream(QDataStream &s);

    JZProjectItem *m_parent;
    QList<QSharedPointer<JZProjectItem>> m_childs;
    
    int m_itemType;    
    QString m_name;           
};
typedef QSharedPointer<JZProjectItem> JZProjectItemPtr;

//JZProjectItemRoot
class JZProjectItemRoot : public JZProjectItem
{
public:
    JZProjectItemRoot();
    virtual ~JZProjectItemRoot();

    JZProject *rootProject();
    void setRootProject(JZProject *project);

protected:
    JZProject *m_project;
};

//JZProjectItemFolder
class JZProjectItemFolder : public JZProjectItem
{
public:
    JZProjectItemFolder();
    virtual ~JZProjectItemFolder();
};

int JZProjectItemIsScript(JZProjectItem *item);


typedef JZProjectItem *(*JZProjectItemCreateFunc)();
template<class T>
JZProjectItem *createJZProjectItem(){ return new T(); }

class JZProjectItemManager
{
public:
    static JZProjectItemManager *instance();

    void registItem(int item_type,JZProjectItemCreateFunc);
    JZProjectItem *create(int item_type);

protected:
    QMap<int,JZProjectItemCreateFunc> m_funcs;    
};

#endif
