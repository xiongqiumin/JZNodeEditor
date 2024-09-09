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
#include "JZNodeVariableBind.h"

//JZNodeParamBindEditDialog
JZNodeParamBindEditDialog::JZNodeParamBindEditDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::JZNodeParamBindEditDialog();
    ui->setupUi(this);

    ui->boxDir->addItem("UiToData");
    ui->boxDir->addItem("DataToUi");
    ui->boxDir->addItem("Duplex");
}

void JZNodeParamBindEditDialog::init(QString widget)
{
    
}

void JZNodeParamBindEditDialog::setParamBind(JZNodeParamBind bind)
{
    m_bind = bind;    
    ui->boxWidget->setCurrentText(bind.variable);
}

JZNodeParamBind JZNodeParamBindEditDialog::paramBind()
{
    return m_bind;
}

JZNodeParamBindEditDialog::~JZNodeParamBindEditDialog()
{
    delete ui;
}

void JZNodeParamBindEditDialog::on_btnOk_clicked()
{
    m_bind.variable = ui->boxWidget->currentText();

    int index = ui->boxDir->currentIndex();
    if (index == 0)
        m_bind.dir = JZNodeParamBind::UiToData;
    else if (index == 1)
        m_bind.dir = JZNodeParamBind::DataToUi;
    else
        m_bind.dir = JZNodeParamBind::Duplex;

    QDialog::accept();
}

void JZNodeParamBindEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}