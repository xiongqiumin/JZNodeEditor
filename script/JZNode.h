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
    Node_setParam,
    Node_setParamData,
    Node_literal,
    Node_functionStart,
    Node_function,        
    Node_event,
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
    Node_if,                
    Node_parallel,    
    Node_view,        
    Node_print,
    Node_switch,        
    Node_return,
    Node_exit,
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
    int paramIn(int index);
    JZNodeGemo paramInGemo(int index);
    int paramInCount();    
    QVector<int> paramInList();
    int addParamOut(QString name,int extFlag = 0);
    int paramOut(int index);
    JZNodeGemo paramOutGemo(int index);
    int paramOutCount();
    QVector<int> paramOutList();
    
    int addFlowIn();    
    int flowIn();
    JZNodeGemo flowInGemo();
    int addFlowOut(QString name = QString());
    int flowOut(int index = 0);
    JZNodeGemo flowOutGemo(int index = 0);
    QVector<int> flowOutList();
    int flowOutCount();    

    int addSubFlowOut(QString name);
    int addSubFlow(const JZNodePin &prop);
    int subFlowOut(int index);
    JZNodeGemo subFlowOutGemo(int index);
    QVector<int> subFlowList();
    int subFlowCount();
    
    QVariant propValue(int prop) const;
    void setPropValue(int prop,QVariant value);
    virtual QList<int> propType(int idx);
    virtual QMap<int,int> calcPropOutType(const QMap<int,int> &inType);

    virtual void expandNode();
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) = 0;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);    

protected:     
    int m_id;
    int m_type;
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
    int m_flowIn;
};

class JZNodeBreak : public JZNode
{
public:
    JZNodeBreak();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
    int m_flowIn;    
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
    int m_flowIn;    
    QList<int> m_flowOut;
    int m_flowComplete;
};

class JZNodeParallel : public JZNode
{
public:
    JZNodeParallel();

    void addOutPin();

protected:
    int m_flowIn;    
    QList<int> m_flowOut;
    int m_flowComplete;    
};


class JZNodeFor: public JZNode
{
public:
    JZNodeFor();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
    int m_indexStart;
    int m_indexEnd;
    int m_indexOut;

    int m_flowBody;    
    int m_flowIn;        
    int m_flowComplete;    
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
    int cond() const;

protected:
    int m_cond;    
};

class JZNodeIf : public JZNode
{
public:
    JZNodeIf();

    void addCondPin();
    void addElsePin();

protected:
    QList<int> m_cond;    
    QList<int> m_flowCond;
    int m_flowElse;
};

class JZNodeBranch : public JZNode
{
public:
    JZNodeBranch();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
    int m_cond;    
    int m_flowIn; 
    int m_flowOut; 
};

#endif
