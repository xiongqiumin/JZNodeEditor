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

enum BreakPointChange
{
    BreakPoint_add,
    BreakPoint_remove,
};

//BreakPoint
class JZCORE_EXPORT BreakPoint
{
public: 
    enum Type{
        none,
        nodeEnter,    //为nodeId中断
        print,  
    };

    BreakPoint();

    Type type;
    QString file;
    int nodeId;
};
void operator<<(QDataStream &s, const BreakPoint &param);
void operator>>(QDataStream &s, BreakPoint &param);

//JZProject
class JZCORE_EXPORT JZProject : public QObject
{
    Q_OBJECT

public:
    static void setActive(JZProject *project);
    static JZProject* active();

    JZProject();
    ~JZProject();

    bool isNull() const;
    void clear();
    
    void initEmpty();
    bool initConsole();
    bool initProject(QString temp);
    bool newProject(QString path,QString name, QString temp);
    bool open(QString filepath);
    void close();
    bool save();    //只保存工程自身，不保存项目文件

    void addTmp(JZProjectItem *item);
    void removeTmp(JZProjectItem *item);
    bool isTmp(JZProjectItem *item);
    bool isFile(JZProjectItem *item);

    void saveTransaction();
    void saveCommit();

    void saveCache(); //保存 breakpoint 之类的设置信息
    void loadCache();

    void registContainer(QString type);
    void unregistContainer(QString type);
    QStringList containerList() const;

    void importModule(QString module);
    void unimportModule(QString module);
    QStringList moduleList() const;

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

    JZScriptClassItem *getClass(QString class_name);
    QStringList classList();

    bool addItem(QString path, JZProjectItem *item);
    void removeItem(QString path);
    JZProjectItem *getItem(QString path);
    QList<JZProjectItem*> itemList(QString path, int type);
    bool saveItem(JZProjectItem *item);
    bool saveItems(QList<JZProjectItem*> item);
    bool saveAllItem();
    void renameItem(JZProjectItem *item, QString name);    

    JZScriptClassItem *getItemClass(JZProjectItem *item);    
    JZProjectItem *getItemFile(JZProjectItem *item);
    JZScriptFile *getScriptFile(JZProjectItem *item);
    
    const JZFunctionDefine *function(QString name);
    JZScriptItem *functionItem(QString function_name);
    QStringList functionList();

    void addBreakPoint(const BreakPoint &pt);
    void removeBreakPoint(QString file,int id);
    bool hasBreakPoint(QString file,int id);
    BreakPoint breakPoint(QString file, int id);
    QList<BreakPoint> breakPoints();

    void onItemChanged(JZProjectItem *item);     

signals:
    void sigDefineChanged();
    void sigItemChanged(JZProjectItem *item);
    void sigBreakPointChanged(BreakPointChange reason,QString file, int id);
    void sigScriptNodeChanged(JZScriptItem *file, int nodeId,const QByteArray &buffer);
    void sigScriptNodeWidgetChanged(JZScriptItem *file, int nodeId, int propId);

protected:
    Q_DISABLE_COPY(JZProject)

    QList<JZProjectItem *> paramDefineList();

    QString domain(JZProjectItem *item);        
    QString dir(const QString &filepath);    
    int indexOfBreakPoint(QString file,int id);

    void registType();
    void unregistType();
            
    QString m_filepath;
    JZProjectItemRoot m_root;
    QList<JZProjectItemPtr> m_tmp;    
    QStringList m_containers;
    QStringList m_modules;

    QMap<JZProjectItem*, QList<BreakPoint>> m_breakPoints;
    
    QString m_error;
    bool m_blockRegist;
    bool m_isSaveCache;
    QList<JZProjectItem*> m_saveCache;  //为了避免每次save写文件

    static JZProject *m_active;
};

#endif
