﻿#include "JZNodeStack.h"
#include <QGroupBox>
#include <QPainter>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipBoard>
#include "UiCommon.h"
#include "JZNodeEngine.h"

JZNodeStack::JZNodeStack(QWidget *parent)
    :QWidget(parent)
{        
    m_table = new QTableWidget();
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({ "序号","位置" });
    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);

    QVBoxLayout *sub_layout = new QVBoxLayout();
    sub_layout->addWidget(m_table);
    this->setLayout(sub_layout);

    m_running = false;
    m_status = Status_none;
    connect(m_table, &QTableWidget::currentItemChanged, this, &JZNodeStack::onStackChanged);
}

JZNodeStack::~JZNodeStack()
{

}

void JZNodeStack::updateStatus()
{
    if (m_running)
    {
        if (statusIsPause(m_status))
            this->setEnabled(true);
        else
            this->setEnabled(false);
    }
    else
    {
        this->setEnabled(true);
        m_table->clearContents();
        m_table->setRowCount(0);
    }
}

void JZNodeStack::setRuntime(JZNodeRuntimeInfo info)
{
    m_status = info.status;
    updateStatus();
    if (!statusIsPause(m_status))
        return;

    m_table->blockSignals(true);
    m_table->clearContents();
    m_table->setRowCount(info.stacks.size());

    for (int i = info.stacks.size() - 1; i >= 0; i--)
    {
        QTableWidgetItem *itemSeq = new QTableWidgetItem();
        itemSeq->setText(QString::number(i + 1));

        QTableWidgetItem *itemName = new QTableWidgetItem();
        auto &s = info.stacks[i];
        QString line = s.file;
        if (!s.function.isEmpty())
        {
            line += "(" + s.function + ")";
        }
        itemName->setText(line);

        int row = info.stacks.size() - i - 1;

        m_table->setItem(row, 0, itemSeq);
        m_table->setItem(row, 1, itemName);
    }
    m_table->blockSignals(false);
    m_table->setCurrentCell(0, 0);
}

void JZNodeStack::setRunning(bool isRun)
{
    m_running = isRun;
    if (!m_running)
        m_status = Status_none;
    updateStatus();
}

void JZNodeStack::onStackChanged()
{
    if (!m_running)
        return;

    int row = m_table->currentRow();
    int level = m_table->rowCount() - row - 1;
    emit sigStackChanged(level);
}