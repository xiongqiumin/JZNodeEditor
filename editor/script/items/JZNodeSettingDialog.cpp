#include "JZNodeSettingDialog.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

//JZNodeManagerDialog
JZNodeManagerDialog::JZNodeManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    m_table = new QTableWidget();

    QVBoxLayout *l = new QVBoxLayout();
    QHBoxLayout *h = new QHBoxLayout();

    QPushButton *btnAdd = new QPushButton("添加");
    QPushButton *btnRemove = new QPushButton("删除");
    QPushButton *btnOk = new QPushButton("确定");
    QPushButton *btnCancel = new QPushButton("取消");
    connect(btnAdd,&QPushButton::clicked,this, &JZNodeManagerDialog::onBtnAddClicked);
    connect(btnRemove,&QPushButton::clicked,this, &JZNodeManagerDialog::onBtnRemoveClicked);
    connect(btnOk,&QPushButton::clicked,this, &JZNodeManagerDialog::onBtnOkClicked);
    connect(btnCancel,&QPushButton::clicked,this, &JZNodeManagerDialog::onBtnCancelClicked);
    h->addStretch();
    h->addWidget(btnAdd);
    h->addWidget(btnRemove);
    h->addWidget(btnOk);
    h->addWidget(btnCancel);
    h->setContentsMargins(0,0,0,0);
    h->setSpacing(3);
    
    l->addWidget(m_table);
    l->addLayout(h);

    setLayout(l);
}

JZNodeManagerDialog::~JZNodeManagerDialog()
{
}

void JZNodeManagerDialog::onBtnAddClicked()
{
}

void JZNodeManagerDialog::onBtnRemoveClicked()
{
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