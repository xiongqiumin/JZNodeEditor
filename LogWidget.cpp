#include "LogWidget.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <QMenu>
#include <QTextDocument>
#include <QScrollBar>

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

LogBrowser::TagInfo LogBrowser::parseTag(QString text)
{
    LogBrowser::TagInfo tg;

    int s1 = text.indexOf("<");
    int e1 = text.indexOf(" ");
    tg.name = text.mid(s1 + 1, e1 - (s1 + 1));
    s1 = e1 + 1;
    e1 = text.indexOf(">");

    QString param_line = text.mid(s1, e1 - s1);
    QStringList lines = param_line.split(" ");
    for (int i = 0; i < lines.size(); i++)
    {
        QString attr = lines[i];
        int idx = attr.indexOf("=");
        tg.params[attr.left(idx)] = attr.mid(idx+1);
    }

    int s2 = text.indexOf("<", e1);
    tg.text = text.mid(e1 + 1, s2 - (e1 + 1));

    return tg;
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

            auto tg = parseTag(link);

            QString pre_text = line.left(s);
            tc.insertBlock();
            tc.insertText(pre_text);

            QTextCharFormat fmt;
            fmt.setForeground(QColor("blue"));
            fmt.setAnchor(true);
            fmt.setAnchorHref(tg.params["href"].toString());
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
        m_logs << edit;

        QWidget *w = new QWidget();
        QVBoxLayout *sub_layout = new QVBoxLayout();        
        sub_layout->addWidget(edit);
        w->setLayout(sub_layout);
        m_tabWidget->addTab(w,domains[i]);

        connect(edit, &LogBrowser::anchorClicked, this, &LogWidget::onAchorClicked);
    }

    m_breakPoint = new JZNodeBreakPoint();
    m_tabWidget->addTab(m_breakPoint, "断点");

    m_stack = new JZNodeStack();
    m_tabWidget->addTab(m_stack, "堆栈");

    m_watchManual = new JZNodeWatch();
    m_tabWidget->addTab(m_watchManual, "监控");

    m_watchAuto = new JZNodeWatch();
    m_watchAuto->setReadOnly(true);
    m_tabWidget->addTab(m_watchAuto, "自动窗口");
}

LogWidget::~LogWidget()
{

}

void LogWidget::clearLog(int type)
{
    m_logs[type]->clear();
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

JZNodeWatch *LogWidget::watchAuto()
{
    return m_watchAuto;
}

JZNodeWatch *LogWidget::watchManual()
{
    return m_watchManual;
}

JZNodeBreakPoint *LogWidget::breakpoint()
{
    return m_breakPoint;
}

void LogWidget::onAchorClicked(QUrl url)
{
    QString link = url.toString();    

    int idx = link.indexOf("?id=");    
    QString file = link.left(idx);
    int node_id = link.mid(idx+4).toInt();
    sigNavigate(file, node_id);
}