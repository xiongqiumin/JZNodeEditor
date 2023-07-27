#include "LogWidget.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <QMenu>

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
        QTextEdit *edit = new QTextEdit();
        m_logs << edit;

        QWidget *w = new QWidget();
        QVBoxLayout *sub_layout = new QVBoxLayout();        
        sub_layout->addWidget(edit);
        w->setLayout(sub_layout);
        m_tabWidget->addTab(w,domains[i]);

        m_logs[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_logs[i], &QWidget::customContextMenuRequested, this, &LogWidget::onLogContextMenu);
    }

    /*
    QTextCharFormat fmt;
    fmt.setForeground(QColor("blue"));
    fmt.setAnchor(true);
    fmt.setAnchorHref("http://example.com");
    fmt.setToolTip("address");
    fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    m_logs[0]->textCursor().insertText("Hello world again\n", fmt);
    */
}

LogWidget::~LogWidget()
{

}

void LogWidget::addLog(int type, const QString &log)
{
    m_logs[type]->append(log);
}

void LogWidget::mousePressEvent(QMouseEvent *event)
{
    int index = m_tabWidget->currentIndex();
    QTextEdit *w = m_logs[index];
    QPoint pt = w->mapFrom(this, event->pos());
    QString text = w->anchorAt(pt);
    qDebug() << text;
}

void LogWidget::onLogContextMenu(QPoint pos)
{
    QTextEdit *edit = (QTextEdit *)sender();

    QMenu menu(edit);
    QAction *actCopy = menu.addAction("复制");
    menu.addSeparator();
    QAction *actClear = menu.addAction("全部清除");    
    

    QAction *act = menu.exec(edit->mapToGlobal(pos));
    if (!act)
        return;

    if (act == actClear)    
        edit->clear();
    else if (act == actCopy)    
        edit->copy();    
}