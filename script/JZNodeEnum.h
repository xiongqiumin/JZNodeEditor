#ifndef JZNODE_ENUM_DEFINE_H_
#define JZNODE_ENUM_DEFINE_H_

#include <QStringList>
#include <QVector>

class JZNodeEnumDefine
{
public:
    void init(QString name, QStringList keys, QVector<int> values);

    void setType(int type);
    int type() const;
    
    QString name() const;
    int count() const;
        
    bool hasKey(const QString &key) const;
    QString key(int index) const;
    QString defaultKey() const;

    int value(int index) const;
    int defaultValue() const;

    int keyToValue(QString key) const;
    QString valueToKey(int value) const;

protected:
    int m_type;
    QString m_name;
    QStringList m_keys;
    QVector<int> m_values;
};



#endif // ! JZNODE_ENUM_H_
