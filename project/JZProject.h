#ifndef JZNODE_PROJECT_H_
#define JZNODE_PROJECT_H_

#include <QSet>
#include <QMap>
#include "JZNode.h"
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
    
    bool initProject(QString temp);
    bool newProject(QString path,QString name, QString temp);
    bool open(QString filepath);
    void close();
    bool save();

    void saveTransaction();
    void saveCommit();

    void saveCache(); //保存 breakpoint 之类的设置信息
    void loadCache();

    void registContainer(QString type);
    void unregistContainer(QString type);
    QStringList containerList() const;

    QString error();

    QString name();    
    QString path();

    void setFilePath(QString path);
    QString filePath();

    JZScriptFile *mainFile();
    QString mainFilePath();

    JZScriptItem *mainFunction(); 
    QString mainFunctionPath();
    
    JZParamItem *globalDefine();
    void addGlobalVariable(const QString &name,QString dataType,const QString &value = QString());
    void addGlobalVariable(const QString &name,int dataType,const QString &value = QString());
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
    JZScriptFile *getScriptFile(JZProjectItem *item);
    
    const JZFunctionDefine *function(QString name);
    QStringList functionList();

    bool hasBreakPoint(QString file,int id);
    void addBreakPoint(QString file,int id);
    void removeBreakPoint(QString file,int id);
    QMap<QString, QList<int>> breakPoints();

    void regist(JZProjectItem *item);     

signals:
    void sigFileChanged();
    void sigScriptNodeChanged(JZScriptItem *file, int nodId,const QByteArray &buffer);

protected:
    Q_DISABLE_COPY(JZProject)

    QList<JZProjectItem *> paramDefineList();    

    QString domain(JZProjectItem *item);        
    void registType();
    QString dir(const QString &filepath);    
            
    JZProjectItemRoot m_root;
    JZProjectItemRoot m_temp;
    
    bool m_windowSystem;        

    QMap<JZProjectItem*, QList<int>> m_breakPoints;
    QString m_filepath;    
    bool m_blockRegist;
    QString m_error;

    bool m_isSaveCache;
    QList<JZProjectItem*> m_saveCache;
    QStringList m_containers;
};

#endif
