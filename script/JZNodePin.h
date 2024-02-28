#ifndef JZNODE_PIN_H_
#define JZNODE_PIN_H_

#include <QVariant>
#include <QPointF>
#include <QDataStream>
#include "JZNodeDefine.h"
#include "JZNodeType.h"

enum
{
    Prop_none,
    Prop_in = 0x1,
    Prop_out = 0x2,    
    Prop_param = 0x4,       //参数
    Prop_flow = 0x8,        //流程 
    Prop_subFlow  = 0x10,   //子程序
    Prop_button = 0x20,
    Prop_dispName = 0x40,
    Prop_editName = 0x80,
    Prop_dispValue = 0x100,
    Prop_editValue = 0x200,
    Prop_noValue = 0x400,
    Prop_literal = 0x800,

    Prop_All = 0xFFFFFFFF,
};

enum {    
    Pri_none = -1,
    Pri_sub_flow = 0,
    Pri_flow = 100,
    Pri_param = 200,
    Pri_button = 300,
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
    int flag() const;

    bool isInput() const;
    bool isOutput() const;

    bool isParam() const;
    bool isFlow() const;
    bool isSubFlow() const;
    bool isButton() const;

    bool isEditValue() const;
    bool isDispName() const;
    bool isDispValue() const;
    bool isLiteral() const;

    void setDataType(QList<int> type);
    QList<int> dataType() const;    

    QString value() const;
    void setValue(QString value);

protected:
    friend void operator<<(QDataStream &s, const JZNodePin &param);
    friend void operator>>(QDataStream &s, JZNodePin &param);    

    int m_id;
    int m_flag;    
    QString m_name;      
    QList<int> m_dataType;
    QString m_value;    
};
void operator<<(QDataStream &s, const JZNodePin &param);
void operator>>(QDataStream &s, JZNodePin &param);

#endif
