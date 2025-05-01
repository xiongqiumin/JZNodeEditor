#ifndef DATA_BASE_H_
#define DATA_BASE_H_

#include <QSqlDatabase>
#include "JZNodeUtils.h"

class DataBaseConfig
{
public:
    DataBaseConfig();
    ~DataBaseConfig();

    void init();

    void addConfig(QString name);

    void setConfigBuffer(QString key,const QByteArray &buffer);
    QByteArray getConfigBuffer(QString key);

protected:
    QSqlDatabase m_db;
};



#endif // !DATA_BASE_H_
