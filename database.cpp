#include "database.h"


DataBaseConfig::DataBaseConfig()
{
}

DataBaseConfig::~DataBaseConfig()
{

}

void DataBaseConfig::init()
{
}

void DataBaseConfig::addConfig(QString name)
{
}

void DataBaseConfig::setConfigBuffer(QString key, const QByteArray &buffer)
{
    QString sql = QString("update ConfigTable set Content='%1' where ConfigID='%2'").arg(text).arg(id);
}

QByteArray DataBaseConfig::getConfigBuffer(QString key)
{
}