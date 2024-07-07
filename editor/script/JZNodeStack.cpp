#include "JZNodeStack.h"
#include <QGroupBox>
#include <QPainter>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipBoard>
#include <QKeyEvent>
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
    m_stackIndex = -1;

    QVBoxLayout *sub_layout = new QVBoxLayout();
    sub_layout->addWidget(m_table);
    this->setLayout(sub_layout);
    
    m_status = Process_none;
    connect(m_table, &QTableWidget::itemDoubleClicked, this, &JZNodeStack::onItemDoubleClicked);
}

JZNodeStack::~JZNodeStack()
{

}

void JZNodeStack::updateStatus()
{
    if (m_status == Process_none)
    {     
        this->setEnabled(true);
        m_table->clearContents();
        m_table->setRowCount(0);
        m_stackIndex = -1;
    }
    else if (m_status == Process_pause)
    {
        this->setEnabled(true);
    }
    else
    {
        this->setEnabled(false);
    }
}

void JZNodeStack::setRuntime(const JZNodeRuntimeInfo &info)
{    
    if (m_status == Process_none)
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
    m_table->setCurrentCell(0, 0);    
    m_stackIndex = 0;
    m_table->blockSignals(false);  
}

void JZNodeStack::setStackIndex(int stack)
{
    m_stackIndex = stack;
}

int JZNodeStack::stackIndex()
{
    return m_stackIndex;
}

void JZNodeStack::setRunningMode(ProcessStatus flag)
{
    m_status = flag;    
    updateStatus();
}

void JZNodeStack::onItemDoubleClicked(QTableWidgetItem *item)
{
    if (m_status != Process_pause)
        return;

    stackChanged(item->row());    
}

void JZNodeStack::stackChanged(int row)
{
    m_stackIndex = m_table->rowCount() - row - 1;    
    emit sigStackChanged(m_stackIndex);
}

void JZNodeStack::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter)
    {
        if (m_status != Process_pause || m_table->currentRow() == -1)
            return;
        
        stackChanged(m_table->currentRow());
    }
}