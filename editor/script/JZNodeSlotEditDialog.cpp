#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include <QHBoxLayout>
#include "JZNodeFunctionManager.h"
#include "JZNodeSlotEditDialog.h"
#include "mainwindow.h"

//JZNodeSlotEditDialog
JZNodeSlotEditDialog::JZNodeSlotEditDialog(QWidget *parent)
    : JZBaseDialog(parent)
{
    m_listParam = new QListWidget();
    m_listSingle = new QListWidget();

    QHBoxLayout *l = new QHBoxLayout();
    l->addWidget(m_listParam);
    l->addWidget(m_listSingle);
    l->setContentsMargins(0,0,0,0);

    m_mainWidget->setLayout(l);

    connect(m_listParam,&QListWidget::currentItemChanged,this,&JZNodeSlotEditDialog::onListParamChanged);
    this->resize(500,320);
}

JZNodeSlotEditDialog::~JZNodeSlotEditDialog()
{
}

void JZNodeSlotEditDialog::onListParamChanged(QListWidgetItem *current)
{
    m_listSingle->clear();
    auto row = m_listParam->currentRow();
    if(row == -1)
        return;

    auto def = m_class.param(current->text());
    auto meta = editorEnvironment()->objectManager()->meta(def->type);
    auto sigs = meta->signalList();
    for(int i = 0; i < sigs.size(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(sigs[i]);
        m_listSingle->addItem(item);
    }
    m_listSingle->setCurrentRow(0);
}

bool JZNodeSlotEditDialog::onOk()
{
    if(!m_listParam->currentItem() || !m_listSingle->currentItem())
        return false;

    m_param = m_listParam->currentItem()->text();
    m_signal = m_listSingle->currentItem()->text();
    return true;
}

QString JZNodeSlotEditDialog::param()
{
    return m_param;
}

QString JZNodeSlotEditDialog::signal()
{
    return m_signal;
}