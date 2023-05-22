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

    void init();
    bool open(QString filepath);
    bool save();    
    bool saveAs(QString filepath);
    QString name();

    void addVariable(QString name,QVariant value);
    void removeVariable(QString name);
    void setVariable(QString name,QVariant value);
    QVariant getVariable(QString name);
    QStringList variableList();
    
    int addItem(QString dir,JZProjectItem *item);
    void removeItem(QString path);
    int renameItem(JZProjectItem *item,QString newname);
    JZProjectItem *getItem(QString path);
    QList<JZProjectItem*> items();
    JZProjectItem *root();

protected:
    Q_DISABLE_COPY(JZProject)

    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);
    void sort();
    void makeTree();        
        
    QList<JZProjectItemPtr> m_items;
    JZProjectRoot m_root;
    QString m_filepath;
    QMap<QString,QVariant> m_variables;
};

#endif
