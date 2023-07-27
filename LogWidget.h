#ifndef LOG_WIDGET_H_
#define LOG_WIDGET_H_

#include <QWidget>
#include <QTextEdit>
#include <QTabWidget>
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

signals:
    void sigNodeClicked(QString file,int id);

protected slots:
    void onLogContextMenu(QPoint pos);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;

    QList<QTextEdit*> m_logs;
    QTabWidget *m_tabWidget;
};





#endif