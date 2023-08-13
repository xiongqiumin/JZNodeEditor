#ifndef JZNODE_ENUM_H_
#define JZNODE_ENUM_H_

#include <QStringList>

class JZNodeEnum
{
public:
    int count();
    QString key(int index);
    int value(int index);

protected:
    QStringList m_keys;
    QVector<int> m_values;
};



#endif // ! JZNODE_ENUM_H_
