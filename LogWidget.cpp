#include "LogWidget.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <QMenu>
#include <QTextDocument>

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
        if (list[i].startsWith("link:"))
        {
            QString line = list[i];
            int s = line.indexOf("(");
            int e = line.indexOf(")");
            QString link = line.mid(s+1, e - s-1);
            
            auto tc = textCursor();            
            tc.movePosition(QTextCursor::End);
            tc.setCharFormat(m_baseForamt);

            tc.insertBlock();
            tc.insertText(line.mid(5,s-5));                        

            QTextCharFormat fmt;
            fmt.setForeground(QColor("blue"));
            fmt.setAnchor(true);
            fmt.setAnchorHref(link);
            fmt.setToolTip("address");
            fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);                        
            tc.insertText(line.mid(s), fmt);            
                        
            setCurrentCharFormat(m_baseForamt);
                        
        }
        else
        {
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

    m_watchReg = new JZNodeWatch();
    m_watchReg->setReadOnly(true);
    m_tabWidget->addTab(m_watchReg, "寄存器");
}

LogWidget::~LogWidget()
{

}

void LogWidget::addLog(int type, const QString &log)
{
    m_logs[type]->addLog(log);
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

JZNodeWatch *LogWidget::watchReg()
{
    return m_watchReg;
}

JZNodeBreakPoint *LogWidget::breakpoint()
{
    return m_breakPoint;
}

void LogWidget::onAchorClicked(QUrl url)
{
    QString link = url.toString();    

    int index_file = link.indexOf(",");    
    QString file = link.mid(0,index_file);    

    int id_s = link.indexOf("id=") + 3;
    int id_e = link.indexOf(")");
    QString node_id_text = link.mid(id_s, id_e - id_s);
    int node_id = node_id_text.toInt();
    sigNodeClicked(file, node_id);
}