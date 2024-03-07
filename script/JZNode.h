#ifndef JZNODE_H_
#define JZNODE_H_

#include <QSharedPointer>
#include <QDataStream>
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
    Node_stringAdd,
    Node_stringEq,
    Node_stringNe,
    Node_stringLe,
    Node_stringGe,
    Node_stringLt,
    Node_stringGt,
    Node_floatEq,
    Node_floatNe,
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
    Node_custom,
};

enum
{
    NodeProp_none = 0,
    NodeProp_noRemove = 0x1,
    NodeProp_dragVariable = 0x2,
};

//JZNodeGemo
struct JZNodeGemo
{
    JZNodeGemo();
    JZNodeGemo(int id, int pin_id);

    bool isNull() const;
    bool operator==(const JZNodeGemo &other) const;

    int nodeId;
    int pinId;
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
void operator<<(QDataStream &s, const JZNodeConnect &param);
void operator>>(QDataStream &s, JZNodeConnect &param);

//JZNodeGroup
class JZNodeGroup
{
public:
    JZNodeGroup();

    int id;
    QString memo;
};
void operator<<(QDataStream &s, const JZNodeGroup &param);
void operator>>(QDataStream &s, JZNodeGroup &param);

class JZNodeCompiler;
class JZScriptItem;
class JZNode
{
public:
    JZNode();
    virtual ~JZNode();

    QByteArray toBuffer();
    bool fromBuffer(const QByteArray &object);

    JZScriptItem *file() const;
    void setFile(JZScriptItem *file);

    const QString &name() const;
    void setName(const QString &name);

    int id() const;
    void setId(int id);
    int type() const;

    void setFlag(int flag);
    int flag() const;

    void setGroup(int group);
    int group();

    const QString &memo() const;
    void setMemo(const QString &text);

    bool isFlowNode() const;    

    int addPin(const JZNodePin &pin);         
    void removePin(int id);
    JZNodePin *pin(int id);
    const JZNodePin *pin(int id) const;
    JZNodePin *pin(QString name);
    bool hasPin(int id) const;
    int indexOfPin(int id) const;
    int indexOfPinByName(QString name) const;
    int indexOfPinByType(int id, int type) const;
    QVector<int> pinInList(int flag = 0) const;
    QVector<int> pinOutList(int flag = 0) const;
    QVector<int> pinListByType(int flag) const;    
    QVector<int> pinList() const;
    int pinCount(int flag) const;
    virtual int pinPri(int id) const;
              
    int addParamIn(QString name,int extFlag = 0);    
    int paramIn(int index) const;
    JZNodeGemo paramInGemo(int index) const;
    int paramInCount() const;
    QVector<int> paramInList() const;
    QString paramInValue(int index) const;
    void setParamInValue(int index, const QString &value);

    int addParamOut(QString name,int extFlag = 0);
    int paramOut(int index) const;
    JZNodeGemo paramOutGemo(int index) const;
    int paramOutCount() const;
    QVector<int> paramOutList() const;
    QString paramOutValue(int index) const;
    void setParamOutValue(int index, const QString &value);
    
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

    int addWidgetIn(QString name);
    int addWidgetOut(QString name);
    int widgetIn(int index) const;
    int widgetOut(int index) const;
    
    const QString &pinValue(int pin) const;
    void setPinValue(int pin, const QString &value);
    const QString &pinName(int id) const;
    void setPinName(int id,const QString &name);

    bool canRemove();
    bool canDragVariable();    

    virtual QList<int> pinType(int id);
    virtual bool update();              //返回属性是否变化
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) = 0;
    
    virtual void drag(const QVariant &value);

    virtual QWidget *createWidget(int id);    
    virtual QStringList pinActionList(int id);
    virtual bool pinActionTriggered(int id,int index);

protected:     
    friend JZScriptItem;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    void setPinTypeAny(int id);
    void setPinTypeInt(int id);
    void setPinTypeNumber(int id);
    void setPinTypeBool(int id);
    void setPinTypeString(int id);
    void setPinType(int id,const QList<int> &type);
    void clearPinType(int id);

    virtual void fileInitialized();
    virtual void pinLinked(int id);
    virtual void pinUnlinked(int id);
    virtual void pinChanged(int id);
    void propertyChangedNotify(const QByteArray &old);

    int m_id;
    int m_type;
    int m_flag;
    int m_group;
    QString m_name;
    QString m_memo;
    QList<JZNodePin> m_pinList;
    JZScriptItem *m_file;
    QList<int> m_notifyList;
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

    void setFunction(const JZFunctionDefine *def);
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
    virtual QWidget *createWidget(int id) override;

    int addSequeue();
    void removeSequeue(int id);    

protected:

};

//JZNodeParallel
class JZNodeParallel : public JZNode
{
public:
    JZNodeParallel();    

protected:   
};

//JZNodeFor
class QComboBox;
class JZNodeFor: public JZNode
{
public:
    JZNodeFor();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual QWidget* createWidget(int id) override;

    void setRange(int start, int end);
    void setRange(int start, int step, int end);
    void setStart(int start);
    void setStep(int step);
    void setEnd(int end);
    void setOp(int op);

protected:    
    virtual void loadFromStream(QDataStream &s) override;
    void updateWidget();

    QStringList m_condTip;
    QList<int> m_condOp;    
    QComboBox *m_comboBox;
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
    virtual QWidget* createWidget(int id) override;
    virtual QStringList pinActionList(int id);
    virtual bool pinActionTriggered(int id, int index);
    void updateCondName();
    int pinPri(int id);

    int btnCondId();
    int btnElseId();    
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

    void setCaseValue(int index, const QString &v);

protected:
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;    
    virtual QWidget* createWidget(int id) override;
    virtual QStringList pinActionList(int id);
    virtual bool pinActionTriggered(int id, int index);
    virtual int pinPri(int id);

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
