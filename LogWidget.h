#ifndef LOG_WIDGET_H_
#define LOG_WIDGET_H_

#include <QWidget>
#include <QTextEdit>
#include "UiCommon.h"

enum {
    Log_Compiler,
    Log_Runtime,
};

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    LogWidget();
    ~LogWidget();

    void addLog(int type, const QString &log);

protected:
    QList<QTextEdit*> m_logs;
};





#endif