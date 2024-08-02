#include <QDateTime>
#include <QDebug>
#include "JZTx.h"

ScopedRecord::ScopedRecord(QString name)
{
    m_name = name;
    m_time = QDateTime::currentMSecsSinceEpoch();
}

ScopedRecord::~ScopedRecord()
{
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    qDebug().noquote() << m_name + ": " + QString::number(time - m_time) + "ms";
}