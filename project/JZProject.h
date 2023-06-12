#ifndef JZNODE_PROJECT_H_
#define JZNODE_PROJECT_H_

#include "JZNode.h"
#include <QMap>
#include "JZNodeObject.h"
#include "JZProjectItem.h"
#include "JZParamFile.h"
#include "JZScriptFile.h"

class JZProject
{
public:
    JZProject();
    ~JZProject();

    void initUi();
    void initConsole();

    bool open(QString filepath);
    bool save();    
    bool saveAs(QString filepath);
    QString name();
    QString mainScript();

    JZScriptFile *addFunction(const FunctionDefine &func);
    void removeFunction(QString name);
    JZScriptFile *getFunction(QString name);

    JZScriptLibraryFile *addLibrary(QString name);
    void removeLibrary(QString name);
    JZScriptLibraryFile *getLibrary(QString libraryName);

    JZScriptClassFile *addClass(QString def,QString super = QString());
    JZScriptClassFile *addUiClass(QString def);
    void removeClass(QString name);
    JZScriptClassFile *getClass(QString className);

    JZParamDefine *getVariableInfo(QString name);
    void setVariable(QString name,const QVariant &value);    
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
    void init();
            
    JZProjectItemFolder m_root;
    QString m_filepath;        
};

#endif
