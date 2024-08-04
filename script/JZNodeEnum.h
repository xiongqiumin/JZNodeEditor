#ifndef JZNODE_ENUM_DEFINE_H_
#define JZNODE_ENUM_DEFINE_H_

#include <QStringList>
#include <QVector>

class JZNodeEnumDefine
{
public:
    void init(QString name, QStringList keys, QVector<int> values);

    void setId(int type);
    int id() const;

    void setFlag(bool isFlag,int enumId);
    bool isFlag() const;
    int flagEnum() const;
    
    QString name() const;
    int count() const;
            
    QString key(int index) const;    
    QString defaultKey() const;
    bool hasKey(const QString &key) const;

    int value(int index) const;    
    int defaultValue() const;
    void setDefaultValue(int value);

    int keyToValue(QString key) const;
    QString valueToKey(int value) const;

protected:
    int m_type;
    QString m_name;
    QStringList m_keys;
    QVector<int> m_values;
    int m_default;
    bool m_isFlag;
    int m_flagEnum;
};



#endif // ! JZNODE_ENUM_H_
