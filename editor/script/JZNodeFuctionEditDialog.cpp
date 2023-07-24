#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include "JZNodeFuctionEditDialog.h"
#include "ui_JZNodeFuctionEditDialog.h"

//JZNodeFuctionEditDialog
JZNodeFuctionEditDialog::JZNodeFuctionEditDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::JZNodeFuctionEditDialog();
    ui->setupUi(this);    

    m_functionDefine.name = "新建函数";
}

JZNodeFuctionEditDialog::~JZNodeFuctionEditDialog()
{
    delete ui;
}

void JZNodeFuctionEditDialog::setFunctionInfo(FunctionDefine info)
{
    m_functionDefine = info;
}

FunctionDefine JZNodeFuctionEditDialog::functionInfo()
{
    return m_functionDefine;
}