#ifndef JZNODE_PROJECT_H_
#define JZNODE_PROJECT_H_

#include "JZNode.h"
#include <QMap>
#include "JZNodeObject.h"
#include "JZProjectItem.h"
#include "JZParamFile.h"
#include "JZScriptFile.h"

//JZProject
class JZProject
{
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

protected:
    Q_DISABLE_COPY(JZProject)

    struct ItemInfo
    {        
        QByteArray buffer;
        QVector<int> breakPoints;
    };
    friend QDataStream &operator<<(QDataStream &s, const ItemInfo &param);
    friend QDataStream &operator >> (QDataStream &s, ItemInfo &param);

    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);
    void itemList(JZProjectItem *item,int type,QList<JZProjectItem *> &list);
    void init();    
    void clear();
    void registType();
    QString dir(const QString &filepath);
            
    JZProjectItemFolder m_root;
    QMap<QString,ItemInfo> m_itemBuffer;
    QString m_filepath;
};

#endif
