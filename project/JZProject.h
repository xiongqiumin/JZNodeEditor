#ifndef JZNODE_PROJECT_H_
#define JZNODE_PROJECT_H_

#include "JZNode.h"
#include <QMap>
#include "JZNodeObject.h"
#include "JZProjectItem.h"
#include "JZParamFile.h"
#include "JZScriptFile.h"
#include "JZClassFile.h"
#include "JZLibraryFile.h"

//JZProject
class JZProject : public QObject
{
    Q_OBJECT

public:
    JZProject();
    ~JZProject();

    bool isVaild();

    void initUi();
    void initConsole();       

    bool open(QString filepath);
    void close();

    bool save();    
    bool saveAs(QString filepath);    

    QString name();
    QString filePath();
    QString path();
    QString mainScript();       

    JZProjectItemFolder *addFolder(QString path,QString name);

    JZScriptFile *addFunction(QString path,const FunctionDefine &func);
    void removeFunction(QString name);
    JZScriptFile *getFunction(QString name);

    JZScriptLibraryFile *addLibrary(QString name);
    void removeLibrary(QString name);
    JZScriptLibraryFile *getLibrary(QString libraryName);

    JZScriptClassFile *addClass(QString path,QString def,QString super = QString());
    JZScriptClassFile *addUiClass(QString path,QString def);
    void removeClass(QString name);
    JZScriptClassFile *getClass(QString className);
    JZScriptClassFile *getClassFile(JZProjectItem *item);

    JZParamDefine *globalVariableInfo(QString name);      
    QStringList globalVariableList();
    
    int addItem(QString dir,JZProjectItem *item);
    void removeItem(QString path);
    JZProjectItem *getItem(QString path);
    void saveItem(QString path);
    void saveItem(JZProjectItem *item);
    void loadItem(JZProjectItem *item);
    void saveAllItem();
    int renameItem(JZProjectItem *item,QString newname);
    QList<JZProjectItem *> itemList(QString path,int type);
    JZProjectItem *root();

    bool hasBreakPoint(QString file,int id);
    void addBreakPoint(QString file,int id);
    void removeBreakPoint(QString file,int id);
    QMap<QString, QVector<int>> breakPoints();

    const FunctionDefine *function(QString name);
    QStringList functionList();

    void regist(JZProjectItem *item);     

signals:
    void sigFileChanged();

protected:
    Q_DISABLE_COPY(JZProject)

    struct ItemInfo
    {        
        QByteArray buffer;
        QVector<int> breakPoints;
    };
    friend QDataStream &operator<<(QDataStream &s, const ItemInfo &param);
    friend QDataStream &operator >> (QDataStream &s, ItemInfo &param);
    QList<JZProjectItem *> paramDefineList();

    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);
    QString domain(JZProjectItem *item);
    void init();    
    void clear();
    void registType();
    QString dir(const QString &filepath);    
            
    JZProjectItemFolder m_root;
    QMap<QString,ItemInfo> m_itemBuffer;
    QString m_filepath;
    bool m_blockRegist;
};

#endif
