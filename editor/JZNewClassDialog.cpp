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
