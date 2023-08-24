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
    Prop_param = 0x4,
    Prop_flow = 0x8,
    Prop_subFlow  = 0x10,   //子程序
    Prop_button = 0x20,
    Prop_dispName = 0x40,
    Prop_editName = 0x80,
    Prop_dispValue = 0x100,
    Prop_editValue = 0x200,
    Prop_literal = 0x400,

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

    void setName(QString name);
    QString name() const;

    void setFlag(int flag);
    int flag() const;

    void setPri(int pri);
    int pri() const; // 显示优先级

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

    QVariant value() const;
    void setValue(QVariant value);

    void addAction(QString action);
    QStringList actionList();

protected:
    friend QDataStream &operator<<(QDataStream &s, const JZNodePin &param);
    friend QDataStream &operator>>(QDataStream &s, JZNodePin &param);

    int m_id;
    int m_flag;
    int m_pri;
    QString m_name;    
    QList<int> m_dataType;
    QVariant m_value;
    QStringList m_actionList;
};
QDataStream &operator<<(QDataStream &s, const JZNodePin &param);
QDataStream &operator>>(QDataStream &s, JZNodePin &param);

#endif
