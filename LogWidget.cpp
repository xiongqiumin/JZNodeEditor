#include "LogWidget.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <QMenu>
#include <QTextDocument>
#include <QScrollBar>
#include <QUrlQuery>
#include "LogManager.h"
#include "JZNodeUtils.h"

//LogBrowser
LogBrowser::LogBrowser()
{    
    setOpenLinks(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &LogBrowser::onLogContextMenu);
    setReadOnly(true);

    m_baseForamt = currentCharFormat();
}

void LogBrowser::onLogContextMenu(QPoint pos)
{
    QMenu menu(this);
    QAction *actCopy = menu.addAction("复制");
    menu.addSeparator();
    QAction *actClear = menu.addAction("全部清除");
    QAction *act = menu.exec(this->mapToGlobal(pos));
    if (!act)
        return;

    if (act == actClear)
        this->clear();
    else if (act == actCopy)
        this->copy();
}

void LogBrowser::addLog(QString log)
{
    QStringList list  = log.split("\n");
    for (int i = 0; i < list.size(); i++)
    {
        QString link_text = "<link";
        if (list[i].contains(link_text))
        {
            QString line = list[i];
            int s = line.indexOf(link_text);
            int e = line.indexOf("</link>");
            QString link = line.mid(s, e - s + 7);
            
            auto tc = textCursor();            
            tc.movePosition(QTextCursor::End);
            tc.setCharFormat(m_baseForamt);

            auto tg = JZNodeUtils::parseLink(link);    

            QString href = tg.params["href"].toString();
            QString pre_text = QUrl(href).path() + ": ";
            tc.insertBlock();
            tc.insertText(pre_text);

            QTextCharFormat fmt;
            fmt.setForeground(QColor("blue"));
            fmt.setAnchor(true);
            fmt.setAnchorHref(href);
            fmt.setToolTip("address");
            fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);                        
            tc.insertText(tg.text, fmt);            
                        
            setCurrentCharFormat(m_baseForamt);                        
        }
        else
        {
            setCurrentCharFormat(m_baseForamt);
            append(list[i]);
        }
    }
}

//LogWidget
LogWidget::LogWidget()
{
    m_tabWidget = new QTabWidget();
    m_tabWidget->setTabPosition(QTabWidget::South);

    QVBoxLayout *l = new QVBoxLayout();    
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_tabWidget);
    this->setLayout(l);

    QStringList domains = { "编译输出","运行输出" };
    for (int i = 0; i < domains.size(); i++)
    {
        LogBrowser *edit = new LogBrowser();
        m_logs[Log_Compiler + i] = edit;

        QWidget *w = new QWidget();
        QVBoxLayout *sub_layout = new QVBoxLayout();        
        sub_layout->addWidget(edit);
        w->setLayout(sub_layout);
        m_tabWidget->addTab(w,domains[i]);

        connect(edit, &LogBrowser::anchorClicked, this, &LogWidget::onAchorClicked);
    }

    m_breakPoint = new JZNodeBreakPointWidget();
    m_tabWidget->addTab(m_breakPoint, "断点");

    m_stack = new JZNodeStack();
    m_tabWidget->addTab(m_stack, "堆栈");

    m_watch = new JZNodeWatch();
    m_tabWidget->addTab(m_watch, "监控");
}

LogWidget::~LogWidget()
{

}

void LogWidget::clearLog(int type)
{
    m_logs[type]->clear();
}

void LogWidget::clearLogs()
{
    auto it = m_logs.begin();
    while (it != m_logs.end())
    {
        it.value()->clear();
        it++;
    }
}

void LogWidget::addLog(int type, const QString &log)
{
    m_logs[type]->addLog(log);
}

void LogWidget::showRunningLog()
{
    m_tabWidget->setCurrentIndex(1);
    m_logs[1]->moveCursor(QTextCursor::End);
}

JZNodeStack *LogWidget::stack()
{
    return m_stack;
}

JZNodeWatch *LogWidget::watch()
{
    return m_watch;
}

JZNodeBreakPointWidget *LogWidget::breakpoint()
{
    return m_breakPoint;
}

void LogWidget::onAchorClicked(QUrl url)
{    
    sigNavigate(url);
}