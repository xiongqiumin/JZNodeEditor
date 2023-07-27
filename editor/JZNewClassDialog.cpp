#include "JZNewClassDialog.h"
#include "ui_JZNewClassDialog.h"

JZNewClassDialog::JZNewClassDialog(QWidget *p)
    :QDialog(p)
{
    ui = new Ui::JZNewClassDialog();
    ui->setupUi(this);
}

JZNewClassDialog::~JZNewClassDialog()
{
    delete ui;
}

QString JZNewClassDialog::className()
{
    return m_className;
}

QString JZNewClassDialog::super()
{
    return m_super;
}

bool JZNewClassDialog::isUi()
{
    return m_isUi;
}

void JZNewClassDialog::on_btnOk_clicked()
{
    QDialog::accepted();
}

void JZNewClassDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}