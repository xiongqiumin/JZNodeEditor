#include "JZNodeBreakPoint.h"
#include <QGroupBox>
#include <QPainter>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipBoard>
#include "UiCommon.h"
#include "JZNodeEngine.h"
#include "mainwindow.h"

JZNodeBreakPoint::JZNodeBreakPoint(QWidget *parent)
    :QWidget(parent)
{
    m_table = new QTableWidget();
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({ "文件","位置" });
    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);    

    QVBoxLayout *sub_layout = new QVBoxLayout();
    sub_layout->addWidget(m_table);
    this->setLayout(sub_layout);
}

JZNodeBreakPoint::~JZNodeBreakPoint()
{

}

void JZNodeBreakPoint::setMainWindow(MainWindow *w)
{
    m_window = w;
}

void JZNodeBreakPoint::updateBreakPoint(JZProject *project)
{
    clear();

    auto breakPoints = project->breakPoints();
    auto it = breakPoints.begin();
    while (it != breakPoints.end())
    {
        for (int i = 0; i < it.value().size(); it++)
        {
            QTableWidgetItem *itemName = new QTableWidgetItem();
            QTableWidgetItem *itemNode = new QTableWidgetItem();
            itemName->setText(it.key());
            itemNode->setText(QString::number(it.value()[i]));

            int row = m_table->rowCount();
            m_table->setRowCount(row + 1);
            m_table->setItem(row, 0, itemName);
            m_table->setItem(row, 1, itemNode);
        }
        it++;
    }
}

void JZNodeBreakPoint::clear()
{
    m_table->clearContents();
    m_table->setRowCount(0);
}