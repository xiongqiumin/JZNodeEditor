#include "JZNodeSettingDialog.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>

//JZNodeManagerDialog
JZNodeManagerDialog::JZNodeManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    m_table = new QTableWidget();
    
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);
    connect(m_table, &QTableWidget::itemDoubleClicked, this, &JZNodeManagerDialog::onItemDoubleClicked);

    QVBoxLayout *l = new QVBoxLayout();
    QHBoxLayout *h = new QHBoxLayout();

    QPushButton *btnAdd = new QPushButton("添加");
    QPushButton *btnRemove = new QPushButton("删除");
    QPushButton *btnSetting = new QPushButton("设置");
    QPushButton *btnOk = new QPushButton("确定");
    QPushButton *btnCancel = new QPushButton("取消");
    
    connect(btnAdd,&QPushButton::clicked,this, &JZNodeManagerDialog::onBtnAddClicked);
    connect(btnRemove,&QPushButton::clicked,this, &JZNodeManagerDialog::onBtnRemoveClicked);
    connect(btnSetting,&QPushButton::clicked,this, &JZNodeManagerDialog::onBtnSettingClicked);
    connect(btnOk,&QPushButton::clicked,this, &JZNodeManagerDialog::onBtnOkClicked);
    connect(btnCancel,&QPushButton::clicked,this, &JZNodeManagerDialog::onBtnCancelClicked);

    h->addStretch();
    h->addWidget(btnAdd);
    h->addWidget(btnRemove);
    h->addWidget(btnSetting);
    h->addWidget(btnOk);
    h->addWidget(btnCancel);
    h->setContentsMargins(0,0,0,0);
    h->setSpacing(3);
    
    l->addWidget(m_table);
    l->addLayout(h);

    setLayout(l);
    resize(600, 400);
}

JZNodeManagerDialog::~JZNodeManagerDialog()
{
}

void JZNodeManagerDialog::onBtnAddClicked()
{
    addConfig();
}

void JZNodeManagerDialog::onBtnRemoveClicked()
{
    int idx = m_table->currentRow();
    if (idx == -1)
        return;

    removeConfig(idx);
}

void JZNodeManagerDialog::onItemDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    settingConfig(row);
}

void JZNodeManagerDialog::onBtnSettingClicked()
{
    int idx = m_table->currentRow();
    if (idx == -1)
        return;

    settingConfig(idx);
}

void JZNodeManagerDialog::onBtnOkClicked()
{
    accept();
}

void JZNodeManagerDialog::onBtnCancelClicked()
{
    reject();
}

//JZNodeSettingDialog    
JZNodeSettingDialog::JZNodeSettingDialog(const QJsonObject& json, QWidget *parent) 
    : QDialog(parent)
{    
}

JZNodeSettingDialog::~JZNodeSettingDialog()
{
}

void JZNodeSettingDialog::setValue(const QJsonObject& json)
{

}

QJsonObject JZNodeSettingDialog::value() const
{
    return QJsonObject();
}