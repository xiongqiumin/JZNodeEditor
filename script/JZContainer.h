#ifndef JZCONTAINER_H_
#define JZCONTAINER_H_

#include <QMap>
#include <QVariant>
#include "JZNodeObject.h"

class TemplateInfo
{
public:
    QString name;
    QStringList args;
    QString error;
};

//JZList
class JZList
{
public:
    QString type() const;

    QString valueType;
    QVariantList list;
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

TemplateInfo parseTemplate(QString type);
bool checkContainer(QString type,QString &error);
void registContainer(QString type,int type_id = -1);


#endif