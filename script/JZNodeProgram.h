#ifndef JZNODE_PROGRAM_H_
#define JZNODE_PROGRAM_H_

#include <QThread>
#include "JZNode.h"
#include "JZNodeEvent.h"
#include "JZEvent.h"
#include "JZNodeIR.h"
#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"

enum{    
    Stack_Node = 0,
    Stack_User = 1000000,    

    Reg_Start = 2000000,    
    Reg_Call = Reg_Start,               //函数传递参数, 调用函数时将 RegCall 数据拷贝到 Stack_User
    Reg_Cmp = 2500000,
};

//NodeRange
struct NodeRange
{
    NodeRange();

    int start;
    int end;
};
QDataStream &operator<<(QDataStream &s, const NodeRange &param);
QDataStream &operator>>(QDataStream &s, NodeRange &param);


//NodeInfo
struct NodeInfo
{        
    NodeInfo();

    int indexOfRange(int pc);
    
    int node_id;
    int node_type;
    bool isFlow;       

    QStringList paramIn;
    QList<int> paramInId;

    QStringList paramOut;
    QList<int> paramOutId;

    QList<NodeRange> pcRanges;
};
QDataStream &operator<<(QDataStream &s, const NodeInfo &param);
QDataStream &operator>>(QDataStream &s, NodeInfo &param);

//JZFunction
class JZFunction
{
public:
    QString file;    
    QList<JZParamDefine> localVariables;
};
QDataStream &operator<<(QDataStream &s, const JZFunction &param);
QDataStream &operator>>(QDataStream &s, JZFunction &param);

//JZNodeScript
class JZNodeScript
{    
public:    
    JZNodeScript();
    void clear();    

    FunctionDefine *function(QString name);

    QString file;
    QString className;
    QMap<int, NodeInfo> nodeInfo;
    QList<JZNodeIRPtr> statmentList;        
    QList<FunctionDefine> functionList;
    QMap<QString, JZFunction> runtimeInfo;

    void saveToStream(QDataStream &s);
    void loadFromStream(QDataStream &s);

protected:
    Q_DISABLE_COPY(JZNodeScript);    
};
typedef QSharedPointer<JZNodeScript> JZNodeScriptPtr;

//JZNodeProgram
class JZNodeBuilder;
class JZNodeProgram 
{
public:
    JZNodeProgram();
    ~JZNodeProgram();

    bool load(QString file);
    bool save(QString file);
    void clear();

    FunctionDefine *function(QString name);
    JZNodeScript *script(QString path);    
    JZFunction *runtimeInfo(QString function);

    QList<JZNodeScript*> scriptList();    
    QMap<QString, QString> bindInfo(QString className);

    QList<FunctionDefine> functionDefines();
    QList<JZNodeObjectDefine> objectDefines();
    QMap<QString,JZParamDefine> globalVariables();    

    QString dump();   
    QString error();
    
protected:
    Q_DISABLE_COPY(JZNodeProgram);

    friend JZNodeBuilder;    
    QString toString(JZNodeIRParam param);
    
    QString m_error;

    QMap<QString,JZNodeScriptPtr> m_scripts; 
    QMap<QString,JZParamDefine> m_variables;        
    QList<JZNodeObjectDefine> m_objectDefines;        
    QMap<QString, QMap<QString, QString>> m_binds;    
};

#endif
