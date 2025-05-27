#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include "JZEventWidget.h"

JZEventWidget::JZEventWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

JZEventWidget::~JZEventWidget()
{
}

void JZEventWidget::setupUI()
{
    tableWidget = new QTableWidget(0, 3, this);
    tableWidget->setHorizontalHeaderLabels({"时间", "类型", "描述"});
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->setVisible(false);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);    
    mainLayout->addWidget(tableWidget);
    setLayout(mainLayout);
}

void JZEventWidget::addEvent(const QString &time, const QString &type, const QString &description)
{
    int row = tableWidget->rowCount();
    tableWidget->insertRow(row);

    tableWidget->setItem(row, 0, new QTableWidgetItem(time));
    tableWidget->setItem(row, 1, new QTableWidgetItem(type));
    tableWidget->setItem(row, 2, new QTableWidgetItem(description));
}

void JZEventWidget::clearEvents()
{
    while (tableWidget->rowCount() > 0) {
        tableWidget->removeRow(0);
    }
}    