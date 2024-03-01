#ifndef LOG_WIDGET_H_
#define LOG_WIDGET_H_

#include <QWidget>
#include <QTextEdit>
#include <QTabWidget>
#include <QTextBrowser>
#include "UiCommon.h"
#include "JZNodeStack.h"
#include "JZNodeWatch.h"
#include "JZNodeBreakPoint.h"

class LogBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    LogBrowser();

    void addLog(QString log);
    
protected slots:
    void onLogContextMenu(QPoint pos);

protected:
    QTextCharFormat m_baseForamt;
};

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    LogWidget();
    ~LogWidget();

    void clearLog(int type);
    void addLog(int type, const QString &log);
    void showRunningLog();

    JZNodeStack *stack();
    JZNodeWatch *watchAuto();
    JZNodeWatch *watchManual();
    JZNodeBreakPoint *breakpoint();

signals:
    void sigNodeClicked(QString file,int id);

protected slots:    
    void onAchorClicked(QUrl url);

protected:
    QList<LogBrowser*> m_logs;
    QTabWidget *m_tabWidget;
    JZNodeStack *m_stack;    
    JZNodeWatch *m_watchAuto;
    JZNodeWatch *m_watchManual;
    JZNodeBreakPoint *m_breakPoint;    
};

#endif