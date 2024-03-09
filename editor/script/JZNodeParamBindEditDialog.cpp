#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include "JZNodeParamBindEditDialog.h"
#include "ui_JZNodeParamBindEditDialog.h"
#include "JZNodeObject.h"
#include "JZNodeTypeHelper.h"
#include "JZClassItem.h"
#include "JZProject.h"

//JZNodeParamBindEditDialog
JZNodeParamBindEditDialog::JZNodeParamBindEditDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::JZNodeParamBindEditDialog();
    ui->setupUi(this);
}

JZNodeParamBindEditDialog::~JZNodeParamBindEditDialog()
{
    delete ui;
}

void JZNodeParamBindEditDialog::on_btnOk_clicked()
{
    QDialog::reject();
}

void JZNodeParamBindEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}