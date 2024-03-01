#ifndef JZNODE_PROJECT_H_
#define JZNODE_PROJECT_H_

#include "JZNode.h"
#include <QMap>
#include "JZNodeObject.h"
#include "JZProjectItem.h"
#include "JZParamItem.h"
#include "JZScriptItem.h"
#include "JZClassItem.h"
#include "JZScriptFile.h"

//JZProject
class JZProject : public QObject
{
    Q_OBJECT

public:
    JZProject();
    ~JZProject();

    bool isNull() const;
    void clear();
    
    bool newProject(QString path,QString name, QString temp);
    bool open(QString filepath);
    void close();
    bool save();        

    void saveCache(); //保存 breakpoint 之类的设置信息
    void loadCache();

    QString error();

    QString name();    
    QString path();

    void setFilePath(QString path);
    QString filePath();

    QString mainFile();
    QString mainScriptPath();
    JZScriptItem *mainScript();
    
    JZParamItem *globalDefine();
    const JZParamDefine *globalVariable(QString name);
    QStringList globalVariableList();

    JZProjectItem *root();

    JZProjectItem *addFile(QString path);
    void removeFile(QString path);    
    void renameFile(QString oldPath,QString newPath);    

    JZScriptClassItem *getClass(QString class_name);

    bool addItem(QString path, JZProjectItem *item);
    void removeItem(QString path);
    JZProjectItem *getItem(QString path);
    QList<JZProjectItem*> itemList(QString path, int type);
    bool saveItem(JZProjectItem *item);
    bool saveItems(QList<JZProjectItem*> item);
    bool saveAllItem();
    bool loadItem(JZProjectItem *item);
    void renameItem(JZProjectItem *item, QString name);    

    JZScriptClassItem *getItemClass(JZProjectItem *item);    
    JZProjectItem *getItemFile(JZProjectItem *item);
    
    const JZFunctionDefine *function(QString name);
    QStringList functionList();

    bool hasBreakPoint(QString file,int id);
    void addBreakPoint(QString file,int id);
    void removeBreakPoint(QString file,int id);
    QMap<QString, QList<int>> breakPoints();

    void regist(JZProjectItem *item);     

signals:
    void sigFileChanged();

protected:
    Q_DISABLE_COPY(JZProject)

    QByteArray magic();
    QList<JZProjectItem *> paramDefineList();    

    QString domain(JZProjectItem *item);        
    void registType();
    QString dir(const QString &filepath);    
            
    JZProjectItemRoot m_root;
    
    bool m_windowSystem;        

    QMap<JZProjectItem*, QList<int>> m_breakPoints;
    QString m_filepath;    
    bool m_blockRegist;
    QString m_error;
};

#endif
