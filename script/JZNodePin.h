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
    Pin_widget = 0x20,
    Pin_dispName = 0x40,    
    Pin_dispValue = 0x80,
    Pin_editValue = Pin_dispValue | 0x100,
    Pin_noValue = 0x200,   //不在运行时起作用
    Pin_literal = 0x400,    
};

enum {    
    Pri_none = -1,
    Pri_sub_flow = 0,
    Pri_flow = 100,
    Pri_param = 200,
    Pri_widget = 300,
};

class JZCORE_EXPORT JZNodePin
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
    void changeFlag(int flag,bool isSet);
    int flag() const;

    bool isInput() const;
    bool isOutput() const;

    bool isParam() const;
    bool isFlow() const;
    bool isSubFlow() const;
    bool isWidget() const;

    bool isEditValue() const;
    bool isDispName() const;
    bool isDispValue() const;
    bool isLiteral() const;

    void setEditType(int edit_type);
    int editType() const;

    void setDataType(const QStringList &type);
    const QStringList &dataType() const;

    const QString &value() const;
    void setValue(const QString &value);
    
protected:
    friend void operator<<(QDataStream &s, const JZNodePin &param);
    friend void operator>>(QDataStream &s, JZNodePin &param);    

    int m_id;
    int m_flag;    
    int m_editType;
    QString m_name;      
    QStringList m_dataType;
    QString m_value;
};
void operator<<(QDataStream &s, const JZNodePin &param);
void operator>>(QDataStream &s, JZNodePin &param);

#endif
