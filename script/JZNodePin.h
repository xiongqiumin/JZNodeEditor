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
    Prop_sub  = 0x10,   //子程序
    Prop_disp = 0x20,
    Prop_edit = 0x40,

    Prop_All = 0xFFFFFFFF,
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

    void setFlag(int dir);
    int flag() const;

    void setDataType(int type);
    int dataType() const;

    void setDefaultValue(QVariant value);
    QVariant defaultValue() const;

protected:
    friend QDataStream &operator<<(QDataStream &s, const JZNodePin &param);
    friend QDataStream &operator>>(QDataStream &s, JZNodePin &param);

    int m_id;
    QString m_name;
    int m_flag;
    int m_dataType;
    QVariant m_default;
};
QDataStream &operator<<(QDataStream &s, const JZNodePin &param);
QDataStream &operator>>(QDataStream &s, JZNodePin &param);

#endif
