#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include "JZNodeParamEditDialog.h"
#include "ui_JZNodeParamEditDialog.h"
#include "JZScriptFile.h"
#include "UiCommon.h"
#include "JZNodeTypeDialog.h"

//JZNodeParamEditDialog
JZNodeParamEditDialog::JZNodeParamEditDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::JZNodeParamEditDialog();
    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));
}

JZNodeParamEditDialog::~JZNodeParamEditDialog()
{
    delete ui;
}

JZParamDefine JZNodeParamEditDialog::param()
{
    return m_paramDefine;
}

void JZNodeParamEditDialog::uiToData()
{
    m_paramDefine.name = ui->lineName->text();
    m_paramDefine.dataType = ui->comboBox->currentData().toInt();
    if (JZNodeType::isBaseType(m_paramDefine.dataType))
    {
        if (ui->lineValue->text().isEmpty())
            m_paramDefine.value = QVariant();
        else
            m_paramDefine.value = ui->lineValue->text();
    }
    else
        m_paramDefine.value = QVariant();
}

void JZNodeParamEditDialog::dataToUi()
{
    ui->lineName->setText(m_paramDefine.name);    

    TypeEditHelp help;
    help.init(m_paramDefine.dataType);
    help.update(ui->comboBox);
    
    if (JZNodeType::isBaseType(m_paramDefine.dataType))
    {
        ui->lineValue->setText(m_paramDefine.value.toString());
        ui->lineValue->setEnabled(true);
    }
    else
        ui->lineValue->setEnabled(false);
}

void JZNodeParamEditDialog::init(JZScriptFile *file, JZParamDefine info)
{    
    m_file = file;
    m_paramDefine = info;
    m_preName = m_paramDefine.name;
    dataToUi();
}

void JZNodeParamEditDialog::onTypeChanged(int index)
{
    QComboBox *box = qobject_cast<QComboBox*>(sender());
    ui->lineValue->setEnabled(index < box->count() - 1);
    if (index < box->count() - 1)   
        return;    

    JZNodeTypeDialog dialog(this);    
    if (dialog.exec() != QDialog::Accepted)
        return;

    TypeEditHelp help;
    help.init(m_paramDefine.dataType);
    help.update(ui->comboBox);
}

void JZNodeParamEditDialog::on_btnOk_clicked()
{
    uiToData();

    if (m_paramDefine.name.isEmpty())
    {
        QMessageBox::information(this, "", "请输入参数名");
        return;
    }

    auto list = m_file->localVariableList(true);    
    if(m_paramDefine.name != m_preName && list.contains(m_paramDefine.name))
    {        
        QMessageBox::information(this, "", "同名参数已存在");
        return;        
    }
    
    QDialog::accept();
}

void JZNodeParamEditDialog::on_btnCancel_clicked()
{
    QDialog::reject();
}