#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include "JZNodeLocalParamEditDialog.h"
#include "ui_JZNodeLocalParamEditDialog.h"


//JZNodeLocalParamEditDialog
JZNodeLocalParamEditDialog::JZNodeLocalParamEditDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::JZNodeLocalParamEditDialog();
    ui->setupUi(this);
}

JZNodeLocalParamEditDialog::~JZNodeLocalParamEditDialog()
{
    delete ui;
}

void JZNodeLocalParamEditDialog::setParam(JZParamDefine define)
{    
}

JZParamDefine JZNodeLocalParamEditDialog::param()
{
    return JZParamDefine();
}

void JZNodeLocalParamEditDialog::on_btnOk_clicked()
{    
    QDialog::accept();
}

void JZNodeLocalParamEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}