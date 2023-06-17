#ifndef JZNODE_H_
#define JZNODE_H_

#include <QDataStream>
#include <QSharedPointer>
#include "JZNodePin.h"
#include "JZNodeIR.h"

enum
{
    Node_none,    
    Node_param,
    Node_create,
    Node_this,
    Node_setParam,
    Node_setParamData,
    Node_literal,
    Node_functionStart,
    Node_function,            
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
    Node_event,
    Node_singleEvent,
    Node_paramChangedEvent,
    Node_timeEvent,
};

enum
{
    Node_propNone = 0,
    Node_propNoRemove = 0x1,
    Node_propVariable = 0x2,
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
class JZNodeGraphItem;
class JZNode
{
public:
    JZNode();
    virtual ~JZNode();

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
    int indexOfProp(int id) const;
    int indexOfPropByName(QString name) const;
    int indexOfPropByType(int id, int type) const;
    QVector<int> propInList(int flag) const;
    QVector<int> propOutList(int flag) const;
    QVector<int> propListByType(int flag) const;    
    QVector<int> propList() const;
    int propCount(int flag) const;
              
    int addParamIn(QString name,int extFlag = 0);    
    int paramIn(int index) const;
    JZNodeGemo paramInGemo(int index) const;
    int paramInCount() const;
    QVector<int> paramInList() const;
    int addParamOut(QString name,int extFlag = 0);
    int paramOut(int index) const;
    JZNodeGemo paramOutGemo(int index) const;
    int paramOutCount() const;
    QVector<int> paramOutList() const;
    
    int addFlowIn();    
    int flowIn() const;
    JZNodeGemo flowInGemo() const;
    int addFlowOut(QString name = QString());
    int flowOut(int index = 0) const;
    JZNodeGemo flowOutGemo(int index = 0) const;
    QVector<int> flowOutList() const;
    int flowOutCount() const;

    int addSubFlowOut(QString name);
    int addSubFlow(const JZNodePin &prop);
    int subFlowOut(int index) const;
    JZNodeGemo subFlowOutGemo(int index) const;
    QVector<int> subFlowList() const;
    int subFlowCount() const;
    
    QVariant propValue(int prop) const;
    void setPropValue(int prop,QVariant value);
    QString propName(int id) const;
    void setPropName(int id,QString name);

    bool canRemove();

    bool hasVariable();
    virtual void setVariable(const QString &name);
    virtual QString variable() const;

    virtual QList<int> propType(int id);
    virtual QMap<int,int> calcPropOutType(const QMap<int,int> &inType);

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) = 0;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);    

protected:     
    void setTypeAny(int id);
    void setTypeInt(int id);
    void setTypeNumber(int id);
    void setTypeBool(int id);
    void setTypeString(int id);

    int m_id;
    int m_type;
    int m_flag;
    QString m_name;
    QList<JZNodePin> m_propList;
};
typedef QSharedPointer<JZNode> JZNodePtr;

class JZStatementNode : public JZNode
{
public:

};

class JZNodeContinue : public JZNode
{
public:
    JZNodeContinue();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

class JZNodeBreak : public JZNode
{
public:
    JZNodeBreak();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:   
};

class JZNodeReturn : public JZNode
{
public:
    JZNodeReturn();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;    

protected:     
};

class JZNodeExit : public JZNode
{
public:
    JZNodeExit();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
    
};

class JZNodeSequence : public JZNode
{
public:
    JZNodeSequence();

    int addSequeue();
    void removeSequeue(int id);

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

class JZNodeParallel : public JZNode
{
public:
    JZNodeParallel();

    void addOutPin();

protected:   
};


class JZNodeFor: public JZNode
{
public:
    JZNodeFor();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

class JZNodeForEach: public JZNode
{
public:
    JZNodeForEach();
    ~JZNodeForEach();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

class JZNodeWhile: public JZNode
{
public:
    JZNodeWhile();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:

};

class JZNodeIf : public JZNode
{
public:
    JZNodeIf();

    void addCondPin();
    void addElsePin();

protected:
};

class JZNodeSwitch : public JZNode
{
public:
    JZNodeSwitch();
};

class JZNodeBranch : public JZNode
{
public:
    JZNodeBranch();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:

};

#endif
