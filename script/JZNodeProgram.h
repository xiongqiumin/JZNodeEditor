#ifndef JZNODE_PROGRAM_H_
#define JZNODE_PROGRAM_H_

#include <QThread>
#include "JZNode.h"
#include "JZNodeEvent.h"
#include "JZEvent.h"
#include "JZNodeIR.h"
#include "JZNodeFunctionDefine.h"

constexpr int STACK_ID_START = 0x1000000;
enum{
    Reg_Start = STACK_ID_START,
    Reg_Jump = Reg_Start,
    Reg_Cmp,
    Reg_User,
    Reg_Call = 0x2000000,   //函数传递参数
};

class JZEventHandle
{
public:
    JZEventHandle();
    
    bool match(JZEvent *event) const;

    int type;    
    int pc;
    QVariantList params;
    FunctionDefine function;
};

class JZNodeBuilder;
class JZNodeProgram 
{
public:
    JZNodeProgram();
    ~JZNodeProgram();

    QString dump();        
    const QList<JZEventHandle> &eventHandleList() const;

    QList<JZNodeIR> opList;
    
protected:
    friend JZNodeBuilder;    
    QString paramName(int id);

    QList<JZEventHandle> m_events;
    QStringList m_opNames;        
};

#endif
