#ifndef JZNODE_H_
#define JZNODE_H_

#include <QDataStream>
#include <QSharedPointer>
#include "JZNodePin.h"
#include "JZNodeIR.h"
#include "JZNodeFunctionDefine.h"

enum
{
    Node_none,    
    Node_nop,
    Node_param,
    Node_create,
    Node_createFromString,
    Node_this,
    Node_setParam,
    Node_setParamData,
    Node_memberParam,
    Node_setMemberParam,
    Node_setMemberParamData,
    Node_literal,
    Node_enum,
    Node_functionStart,
    Node_function,      
    Node_clone,
    Node_assert,
    Node_swap,
    Node_add,
    Node_sub,
    Node_mul,
    Node_div,
    Node_mod,    
    Node_eq,  // ==
    Node_ne,  // !=
    Node_le,  // <=
    Node_ge,  // >=
    Node_lt,  // <
    Node_gt,  // >
    Node_and,
    Node_or,
    Node_not,
    Node_bitand,
    Node_bitor,    
    Node_bitxor,
    Node_expr,
    Node_for,
    Node_foreach,
    Node_while,
    Node_branch,
    Node_sequence,
    Node_if,                
    Node_parallel,    
    Node_view,        
    Node_print,
    Node_switch,
    Node_break,
    Node_continue,
    Node_return,
    Node_exit,
    Node_startEvent,
    Node_singleEvent,
    Node_qtEvent,
    Node_paramChangedEvent,
    Node_timeEvent,   
    Node_display,
};

enum
{
    Node_propNone = 0,
    Node_propNoRemove = 0x1,
    Node_propDragVariable = 0x2,
};

//JZNodeGemo
struct JZNodeGemo
{
    JZNodeGemo();
    JZNodeGemo(int id, int prop);

    bool isNull() const;
    bool operator==(const JZNodeGemo &other) const;

    int nodeId;
    int propId;
};

//JZNodeConnect
class JZNodeConnect
{
public:
    JZNodeConnect();
    
    int id;
    JZNodeGemo from;
    JZNodeGemo to;
};
QDataStream &operator<<(QDataStream &s, const JZNodeConnect &param);
QDataStream &operator>>(QDataStream &s, JZNodeConnect &param);
JZNodeConnect parseLine(const QByteArray &buffer);
QByteArray formatLine(const JZNodeConnect &line);

class JZNodeCompiler;
class JZScriptFile;
class JZNode
{
public:
    JZNode();
    virtual ~JZNode();

    JZScriptFile *file() const;
    void setFile(JZScriptFile *file);

    QString name() const;
    void setName(QString name);

    int id() const;
    void setId(int id);
    int type() const;

    void setFlag(int flag);
    int flag() const;

    bool isFlowNode() const;    

    int addProp(const JZNodePin &prop);         
    void removeProp(int id);
    JZNodePin *prop(int id);
    const JZNodePin *prop(int id) const;
    JZNodePin *prop(QString name);
    bool hasProp(int id) const;
    int indexOfProp(int id) const;
    int indexOfPropByName(QString name) const;
    int indexOfPropByType(int id, int type) const;
    QVector<int> propInList(int flag = 0) const;
    QVector<int> propOutList(int flag = 0) const;
    QVector<int> propListByType(int flag) const;    
    QVector<int> propList() const;
    int propCount(int flag) const;
              
    int addParamIn(QString name,int extFlag = 0);    
    int paramIn(int index) const;
    JZNodeGemo paramInGemo(int index) const;
    int paramInCount() const;
    QVector<int> paramInList() const;
    QVariant paramInValue(int index) const;
    void setParamInValue(int index, const QVariant &value);

    int addParamOut(QString name,int extFlag = 0);
    int paramOut(int index) const;
    JZNodeGemo paramOutGemo(int index) const;
    int paramOutCount() const;
    QVector<int> paramOutList() const;
    QVariant paramOutValue(int index) const;
    void setParamOutValue(int index, const QVariant &value);
    
    int addFlowIn(int extFlag = 0);
    int flowIn() const;
    JZNodeGemo flowInGemo() const;
    int addFlowOut(QString name = QString(),int extFlag = 0);
    int flowOut(int index = 0) const;
    JZNodeGemo flowOutGemo(int index = 0) const;
    QVector<int> flowOutList() const;
    int flowOutCount() const;

    int addSubFlowOut(QString name,int extFlag = 0);
    int subFlowOut(int index) const;
    JZNodeGemo subFlowOutGemo(int index) const;
    QVector<int> subFlowList() const;
    int subFlowCount() const;

    int addButtonIn(QString name);
    int addButtonOut(QString name);
    
    QVariant propValue(int prop) const;
    void setPropValue(int prop,QVariant value);
    QString propName(int id) const;
    void setPropName(int id,QString name);
    
    virtual void setVariable(const QString &name);
    virtual QString variable() const;
    virtual int variableType() const;

    bool canRemove();
    bool canDragVariable();    

    virtual QList<int> propType(int id);        
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) = 0;

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);    
    virtual void drag(const QVariant &value);

    virtual bool pinClicked(int id);    //返回属性是否变化
    virtual QStringList pinActionList(int id);
    virtual bool pinActionTriggered(int id,int index);

protected:     
    friend JZScriptFile;

    void setPinTypeAny(int id);
    void setPinTypeInt(int id);
    void setPinTypeNumber(int id);
    void setPinTypeBool(int id);
    void setPinTypeString(int id);
    void setPinType(int id,const QList<int> &type);

    virtual void fileInitialized();
    virtual void pinLinked(int id);
    virtual void pinUnlinked(int id);
    virtual void pinChanged(int id);
    void sortPinByPri();

    int m_id;
    int m_type;
    int m_flag;
    QString m_name;    
    QList<JZNodePin> m_propList;
    JZScriptFile *m_file;
    QVector<int> m_notifyList;
};
typedef QSharedPointer<JZNode> JZNodePtr;

//JZNodeNop
class JZNodeNop : public JZNode
{
public:
    JZNodeNop();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:
};

//JZNodeContinue
class JZNodeContinue : public JZNode
{
public:
    JZNodeContinue();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

//JZNodeBreak
class JZNodeBreak : public JZNode
{
public:
    JZNodeBreak();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:   
};

//JZNodeReturn
class JZNodeReturn : public JZNode
{
public:
    JZNodeReturn();

    void setFunction(const FunctionDefine *def);
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;    

protected:     
};

//JZNodeExit
class JZNodeExit : public JZNode
{
public:
    JZNodeExit();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
    
};

//JZNodeSequence
class JZNodeSequence : public JZNode
{
public:
    JZNodeSequence();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual bool pinClicked(int id);

    int addSequeue();
    void removeSequeue(int id);    

protected:

};

//JZNodeParallel
class JZNodeParallel : public JZNode
{
public:
    JZNodeParallel();

    void addOutPin();

protected:   
};

//JZNodeFor
class JZNodeFor: public JZNode
{
public:
    JZNodeFor();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    void setRange(int start, int end);
    void setRange(int start, int step, int end);
    void setStart(int start);
    void setStep(int step);
    void setEnd(int end);

protected:
};

//JZNodeForEach
class JZNodeForEach: public JZNode
{
public:
    JZNodeForEach();
    ~JZNodeForEach();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

//JZNodeWhile
class JZNodeWhile: public JZNode
{
public:
    JZNodeWhile();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:

};

//JZNodeIf
class JZNodeIf : public JZNode
{
public:
    JZNodeIf();

    void addCondPin();
    void addElsePin();    
    void removeCond(int index);
    void removeElse();

protected:
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    virtual bool pinClicked(int id) override;
    virtual QStringList pinActionList(int id);
    virtual bool pinActionTriggered(int id, int index);
    void updateCondName();

    int m_btnCond;
    int m_btnElse;
};

//JZNodeSwitch
class JZNodeSwitch : public JZNode
{
public:
    JZNodeSwitch();
    void addCase();
    void addDefault();
    void removeCase(int index);
    void removeDefault();
    int caseCount();

    void setCaseValue(int index, const QVariant &v);

protected:
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    virtual bool pinClicked(int id) override;
    virtual QStringList pinActionList(int id);
    virtual bool pinActionTriggered(int id, int index);

    int m_btnCase;
    int m_btnDefault;
};


//JZNodeBranch
class JZNodeBranch : public JZNode
{
public:
    JZNodeBranch();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:

};

//JZNodeAssert
class JZNodeAssert : public JZNode
{
public:
    JZNodeAssert();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:

};

//JZNodeTryCatch
class JZNodeTryCatch : public JZNode
{
public:
    JZNodeTryCatch();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:
};

#endif
