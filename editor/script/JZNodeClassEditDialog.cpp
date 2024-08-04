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
    ui->lineUi->setText(class_item->uiFile());
}

QString JZNodeClassEditDialog::className()
{
    return ui->lineClassName->text().trimmed();
}

QString JZNodeClassEditDialog::super()
{
    return ui->lineBaseClass->text().trimmed();
}

QString JZNodeClassEditDialog::uiFile()
{
    return ui->lineUi->text().trimmed();
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