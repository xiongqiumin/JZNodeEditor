#ifndef JZNODE_PIN_H_
#define JZNODE_PIN_H_

#include <QVariant>
#include <QPointF>
#include <QDataStream>
#include "JZNodeType.h"

enum
{
    Pin_none,
    Pin_in = 0x1,
    Pin_out = 0x2,
    Pin_param = 0x4,       //参数
    Pin_flow = 0x8,        //流程 
    Pin_subFlow  = 0x10,   //子程序
    Pin_constValue = 0x20,   //需要常数  
    Pin_noCompiler = 0x40,   //节点自己处理，不作为input/output
};

class JZNodePin
{
public:
    JZNodePin();
    JZNodePin(QString name, int dataType, int flag);
    ~JZNodePin();

    void setId(int id);
    int id() const;

    void setName(const QString &name);
    const QString &name() const;

    void setFlag(int flag);
    void changeFlag(int flag,bool isSet); //检测flag是否设置
    int flag() const;

    bool isInput() const;
    bool isOutput() const;

    bool isParam() const;
    bool isFlow() const;
    bool isSubFlow() const;
    bool isConstValue() const;

    void setDataType(const QStringList &type);
    const QStringList &dataType() const;

    const QString &value() const;
    void setValue(const QString &value);
    
protected:
    friend void operator<<(QDataStream &s, const JZNodePin &param);
    friend void operator>>(QDataStream &s, JZNodePin &param);    

    int m_id;
    int m_flag;    
    QString m_name;      
    QStringList m_dataType;
    QString m_value;
};
void operator<<(QDataStream &s, const JZNodePin &param);
void operator>>(QDataStream &s, JZNodePin &param);

#endif
