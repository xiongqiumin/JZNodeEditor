#include "JZNodeClassEditDialog.h"
#include "ui_JZNodeClassEditDialog.h"

JZNodeClassEditDialog::JZNodeClassEditDialog(QWidget *p)
    :QDialog(p)
{
    ui = new Ui::JZNodeClassEditDialog();
    ui->setupUi(this);
}

JZNodeClassEditDialog::~JZNodeClassEditDialog()
{
    delete ui;
}

QString JZNodeClassEditDialog::className()
{
    return m_className;
}

QString JZNodeClassEditDialog::super()
{
    return m_super;
}

bool JZNodeClassEditDialog::isUi()
{
    return m_isUi;
}

void JZNodeClassEditDialog::on_btnOk_clicked()
{
    QDialog::accepted();
}

void JZNodeClassEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}