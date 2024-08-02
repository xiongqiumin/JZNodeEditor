#include <QGroupBox>
#include <QPainter>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipBoard>
#include <QMenu>
#include <QRadioButton>
#include <QVBoxLayout>
#include "UiCommon.h"
#include "JZNodeEngine.h"
#include "mainwindow.h"
#include "JZBaseDialog.h"
#include "JZNodeBreakPoint.h"

//JZBaseDialog
class BreakPointDialog : public JZBaseDialog
{
public:
    BreakPointDialog(QWidget *parent)
        :JZBaseDialog(parent)
    {
        QVBoxLayout *l = new QVBoxLayout();
        QRadioButton *btn1 = new QRadioButton("中断");
        QRadioButton *btn2 = new QRadioButton("打印");
        l->setContentsMargins(0, 0, 0, 0);
        l->addWidget(btn1);
        l->addWidget(btn2);
        m_mainWidget->setLayout(l);

        m_radioList << btn1 << btn2;
    }

    void setBreakPoint(BreakPoint pt)
    {
        m_break = pt;
        if (pt.type == BreakPoint::nodeEnter)
            m_radioList[0]->setChecked(true);
        else
            m_radioList[1]->setChecked(true);
    }

    BreakPoint breakPoint()
    {
        BreakPoint pt = m_break;
        if (m_radioList[0]->isChecked())
            pt.type = BreakPoint::nodeEnter;
        else
            pt.type = BreakPoint::print;
        return pt;
    }

protected:
    virtual bool onOk() override
    {
        return true;
    }

    BreakPoint m_break;
    QList<QRadioButton*> m_radioList;
};

//JZNodeBreakPoint
JZNodeBreakPoint::JZNodeBreakPoint(QWidget *parent)
    :QWidget(parent)
{
    m_project = nullptr;
    m_table = new QTableWidget();
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({ "文件","位置" });
    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_table, &QTableWidget::itemDoubleClicked, this, &JZNodeBreakPoint::onItemDoubleClicked);
    connect(m_table, &QTableWidget::customContextMenuRequested, this, &JZNodeBreakPoint::onContexMenu);

    QVBoxLayout *sub_layout = new QVBoxLayout();
    sub_layout->addWidget(m_table);
    this->setLayout(sub_layout);
}

JZNodeBreakPoint::~JZNodeBreakPoint()
{

}

void JZNodeBreakPoint::setProject(JZProject *project)
{
    m_project = project;
}

void JZNodeBreakPoint::updateBreakPoint()
{
    clear();

    auto pt_list = m_project->breakPoints();
    for (int i = 0; i < pt_list.size(); i++)
    {
        QTableWidgetItem *itemName = new QTableWidgetItem();
        QTableWidgetItem *itemNode = new QTableWidgetItem();
        itemName->setText(pt_list[i].file);
        itemNode->setText(QString::number(pt_list[i].nodeId));

        int row = m_table->rowCount();
        m_table->setRowCount(row + 1);
        m_table->setItem(row, 0, itemName);
        m_table->setItem(row, 1, itemNode);
    }
}

void JZNodeBreakPoint::clear()
{
    m_table->clearContents();
    m_table->setRowCount(0);
}


void JZNodeBreakPoint::onItemDoubleClicked()
{
    int row = m_table->currentRow();
    if (row == -1)
        return;

    QString file = m_table->item(row, 0)->text();
    int id = m_table->item(row, 1)->text().toInt();
    emit sigBreakPointClicked(file,id);
}

void JZNodeBreakPoint::onContexMenu(QPoint pt)
{
    int row = m_table->currentRow();
    if (row == -1)
        return;

    QMenu menu(this);

    auto act_set = menu.addAction("设置");
    auto ret = menu.exec(m_table->mapToGlobal(pt));
    if (!ret)
        return;

    if (ret == act_set)
    {
        QString file = m_table->item(row, 0)->text();
        int id = m_table->item(row, 1)->text().toInt();
        auto bt = m_project->breakPoint(file, id);

        BreakPointDialog dlg(this);
        dlg.setBreakPoint(bt);
        if (dlg.exec() != QDialog::Accepted)
            return;

        m_project->removeBreakPoint(file, id);
        m_project->addBreakPoint(dlg.breakPoint());        
    }
}