﻿#ifndef JZPROJECT_ITEM_FILE_H_
#define JZPROJECT_ITEM_FILE_H_
#include <QSharedPointer>

enum{    
    ProjectItem_root,
    ProjectItem_folder,
    ProjectItem_class,
    ProjectItem_library,
    ProjectItem_ui,
    ProjectItem_param,    
    ProjectItem_scriptParamBinding,
    ProjectItem_scriptFlow,    
    ProjectItem_scriptFunction,

    ProjectItem_any = 0xFFFF,
};

class JZProject;
class JZScriptClassFile;
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

    JZProjectItem *parent();
    
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
