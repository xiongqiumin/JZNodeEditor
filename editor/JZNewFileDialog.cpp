#include "JZNewFileDialog.h"
#include "ui_JZNewFileDialog.h"

JZNewFileDialog::JZNewFileDialog(QWidget *p)
    :QDialog(p)
{
    ui = new Ui::JZNewFileDialog();
    ui->setupUi(this);

    ui->listWidget->addItem("ui class");
    ui->listWidget->addItem("script");
    ui->listWidget->addItem("ui");
}

JZNewFileDialog::~JZNewFileDialog()
{
    delete ui;
}

QString JZNewFileDialog::name()
{
    return ui->lineName->text();
}

QString JZNewFileDialog::path()
{
    return ui->lineDir->text();
}

QString JZNewFileDialog::type()
{
    return ui->listWidget->currentItem()->text();
}

void JZNewFileDialog::on_btnOk_clicked()
{
    QDialog::accepted();
}

void JZNewFileDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}