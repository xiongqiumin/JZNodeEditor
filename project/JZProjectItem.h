#ifndef JZPROJECT_ITEM_FILE_H_
#define JZPROJECT_ITEM_FILE_H_
#include <QSharedPointer>

enum{    
    ProjectItem_none = 0x0,
    ProjectItem_root = 0x1,
    ProjectItem_folder = 0x2,
    ProjectItem_class = 0x4,
    ProjectItem_library = 0x8,
    ProjectItem_ui = 0x10,
    ProjectItem_param = 0x20,
    ProjectItem_scriptParamBinding = 0x40,
    ProjectItem_scriptFlow = 0x80,
    ProjectItem_scriptFunction = 0x100,
    ProjectItem_script = (ProjectItem_scriptParamBinding | ProjectItem_scriptFlow | ProjectItem_scriptFunction),

    ProjectItem_any = 0xFFFFFFFF,
};

class JZProject;
class JZScriptClassFile;
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
    JZScriptClassFile *getClassFile();
    QString className();
    
    void addItem(QSharedPointer<JZProjectItem> child);
    void removeItem(int index);    
    JZProjectItem *getItem(QString name);     
    bool hasItem(QString name);
    int indexOfItem(JZProjectItem *item);
    QList<JZProjectItem *> childs();
    void removeChlids();

    void save();  //保存到project缓存
    void load();  //从project缓存加载
    virtual void saveToStream(QDataStream &s);
    virtual void loadFromStream(QDataStream &s);

    void sort();
    QList<JZProjectItem *> itemList(int type);
    
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
    static JZProjectItem *load(const QByteArray &buffer);
    static QByteArray save(JZProjectItem *item);
    static QString itemTypeName(int itemType);
};

#endif
