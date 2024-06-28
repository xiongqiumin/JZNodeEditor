#ifndef JZCONTAINER_H_
#define JZCONTAINER_H_

#include <QMap>
#include <QVariant>
#include "JZNodeObject.h"

//JZList
class JZList
{
public:
    QString type() const;

    QString valueType;
    QVariantList list;
};

//JZListIterator
class JZListIterator
{
public:
    JZListIterator();

    JZList *list;
    int index;
};

//JZMap
class JZMap
{
public:
    struct Key
    {
        bool operator<(const Key &other) const;

        QVariant v;
    };

    QString type() const;
    
    QString valueType;
    QString keyType;
    QMap<Key,QVariant> map;
};

//JZMapIterator
class JZMapIterator
{
public:
    void next();
    bool atEnd();
    QVariant key();
    QVariant value();

    JZMap *map;
    QMap<JZMap::Key,QVariant>::Iterator it;
};

void registList(QString type,int type_id = -1);
void registMap(QString key_type,QString value_type,int type_id = -1);




#endif