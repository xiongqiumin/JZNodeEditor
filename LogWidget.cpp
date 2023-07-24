#include "LogWidget.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

LogWidget::LogWidget()
{
    QTabWidget *tab = new QTabWidget();
    tab->setTabPosition(QTabWidget::South);

    QVBoxLayout *l = new QVBoxLayout();    
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(tab);
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
        tab->addTab(w,domains[i]);
    }
}

LogWidget::~LogWidget()
{

}

void LogWidget::addLog(int type, const QString &log)
{
    m_logs[type]->append(log);
}