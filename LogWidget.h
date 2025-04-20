#ifndef LOG_WIDGET_H_
#define LOG_WIDGET_H_

#include <QWidget>
#include <QTextEdit>
#include <QTabWidget>
#include <QTextBrowser>
#include "UiCommon.h"
#include "JZNodeStack.h"
#include "JZNodeWatch.h"
#include "JZNodeBreakPointWidget.h"

class LogBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    LogBrowser();

    void addLog(QString log);
    
protected slots:
    void onLogContextMenu(QPoint pos);

protected:
    struct TagInfo {
        QString name;
        QString text;
        QVariantMap params;
    };

    TagInfo parseTag(QString line);

    QTextCharFormat m_baseForamt;
};

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    LogWidget();
    ~LogWidget();

    void clearLog(int type);
    void clearLogs();

    void addLog(int type, const QString &log);
    void showRunningLog();

    JZNodeStack *stack();    
    JZNodeWatch *watch();
    JZNodeBreakPointWidget *breakpoint();

signals:
    void sigNavigate(QUrl url);

protected slots:    
    void onAchorClicked(QUrl url);

protected:    
    QMap<int,LogBrowser*> m_logs;
    QTabWidget *m_tabWidget;
    JZNodeStack *m_stack;        
    JZNodeWatch *m_watch;
    JZNodeBreakPointWidget *m_breakPoint;    
};

#endif