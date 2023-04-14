#ifndef JZNODE_PROGRAM_H_
#define JZNODE_PROGRAM_H_

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
    Reg_Call = 0x2000000,
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

class JZNodeCompiler;
class JZNodeProgram
{
public:
    JZNodeProgram();
    ~JZNodeProgram();

    QString dump();

    QList<JZNodeIR> opList;
    const QList<JZEventHandle> &eventHandleList() const;
    
    QString paramName(int id);
    JZNodeGemo paramGemo(int id);
    int paramId(int nodeId,int propId);
    int paramId(const JZNodeGemo &gemo);

protected:
    friend JZNodeCompiler;
    
    QList<JZEventHandle> m_events;
    QStringList m_opNames;
};

#endif
