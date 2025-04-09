#include <QMessageBox>
#include "JZNodeClassEditDialog.h"
#include "ui_JZNodeClassEditDialog.h"
#include "JZRegExpHelp.h"

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

void JZNodeClassEditDialog::setClass(JZScriptClassItem *class_item)
{
    ui->lineClassName->setText(class_item->name());
    ui->lineBaseClass->setText(class_item->superClass());
    ui->lineUi->setChecked(class_item->hasUi());
}

QString JZNodeClassEditDialog::className()
{
    return ui->lineClassName->text().trimmed();
}

QString JZNodeClassEditDialog::super()
{
    return ui->lineBaseClass->text().trimmed();
}

bool JZNodeClassEditDialog::isUi()
{
    return ui->lineUi->isChecked();
}

void JZNodeClassEditDialog::on_btnOk_clicked()
{
    if (!JZRegExpHelp::isIdentify(className()))
    {
        QMessageBox::information(this, "", "类名错误，请检查");
        return;
    }

    QDialog::accept();
}

void JZNodeClassEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}