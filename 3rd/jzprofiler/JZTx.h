#ifndef JZTX_H_
#define JZTX_H_

#include <QString>

class ScopedRecord
{
public:
    ScopedRecord(QString name);
    ~ScopedRecord();

protected:
    QString m_name;
    qint64 m_time;
};

#define JZTX_FUNCTION ScopedRecord __rec__(__FUNCTION__);


#endif // !JZTX_H_
