#ifndef JZNODE_PROJECT_H_
#define JZNODE_PROJECT_H_

#include "JZNode.h"
#include <QMap>
#include "JZProjectItem.h"
#include "JZNodeObject.h"

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

    void registObject(JZNodeObjectDefine def,QString super = QString());    
    void unregistObject(QString name);    

    void addVariable(QString name,QVariant value);        
    void removeVariable(QString name);
    void setVariable(QString name,QVariant value);

    void addClassVariable(QString name,QString className);
    QString getClassVariable(QString name);

    QVariant getVariable(QString name);
    QStringList variableList();
    
    int addItem(QString dir,JZProjectItem *item);
    void removeItem(QString path);
    int renameItem(JZProjectItem *item,QString newname);
    QList<JZProjectItem *> itemList(QString path,int type);
    JZProjectItem *getItem(QString path);    
    JZProjectItem *root();

    const FunctionDefine *function(QString name);

protected:
    Q_DISABLE_COPY(JZProject)

    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);
    void itemList(JZProjectItem *item,int type,QList<JZProjectItem *> &list);
            
    JZProjectItemFolder m_root;
    QString m_filepath;
    QMap<QString,QVariant> m_variables;
};

#endif
