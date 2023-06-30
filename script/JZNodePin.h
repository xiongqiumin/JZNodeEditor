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

class JZParamDefine
{
public:
    JZParamDefine();
    JZParamDefine(QString name,int dataType,const QVariant &v = QVariant());

    QString name;
    int dataType;
    bool cref;
    QVariant value;
};
QDataStream &operator<<(QDataStream &s, const JZParamDefine &param);
QDataStream &operator>>(QDataStream &s, JZParamDefine &param);

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

    void setFlag(int dir);
    int flag() const;
    int pri() const; // 显示优先级

    bool isInput() const;
    bool isOutput() const;

    bool isParam() const;
    bool isFlow() const;
    bool isSubFlow() const;
    bool isButton() const;

    bool isEditable() const;
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
    QString m_name;    
    QList<int> m_dataType;
    QVariant m_value;
    QStringList m_actionList;
};
QDataStream &operator<<(QDataStream &s, const JZNodePin &param);
QDataStream &operator>>(QDataStream &s, JZNodePin &param);

#endif
