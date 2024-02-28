#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include "JZNodeParamEditDialog.h"
#include "ui_JZNodeParamEditDialog.h"
#include "JZScriptItem.h"
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
    m_paramDefine.type = ui->comboBox->currentText();

    int dataType = m_paramDefine.dataType();
    if (JZNodeType::isBase(dataType))
    {
        if (ui->lineValue->text().isEmpty())
            m_paramDefine.value.clear();
        else
            m_paramDefine.value = ui->lineValue->text();
    }
    else if (JZNodeType::isEnum(dataType))
    {
        m_paramDefine.value = ui->boxValue->currentText();
    }
    else
        m_paramDefine.value.clear();
}

void JZNodeParamEditDialog::dataToUi()
{
    ui->lineName->setText(m_paramDefine.name);   

    int dataType = m_paramDefine.dataType();
    ui->boxValue->setVisible(JZNodeType::isEnum(dataType));
    ui->lineValue->setVisible(!JZNodeType::isEnum(dataType));

    TypeEditHelp help;
    //help.init(m_paramDefine.dataType);
    help.update(ui->comboBox);
    
    if (JZNodeType::isBase(dataType))
    {
        ui->lineValue->setText(m_paramDefine.value);
        ui->lineValue->setEnabled(true);
    }
    else if (JZNodeType::isEnum(dataType))
    {
        UiHelper::updateEnumBox(ui->boxValue, dataType, m_paramDefine.value.toInt());
    }
    else
        ui->lineValue->setEnabled(false);
}

void JZNodeParamEditDialog::init(JZScriptItem *file, JZParamDefine info)
{    
    m_file = file;
    m_paramDefine = info;
    m_preName = m_paramDefine.name;
    dataToUi();
}

void JZNodeParamEditDialog::onTypeChanged(int index)
{
    QComboBox *box = qobject_cast<QComboBox*>(sender());    
    if (index < box->count() - 1)   
        return;    
/*
    JZNodeTypeDialog dialog(this);
    dialog.setDataType(m_paramDefine.type);
    if (dialog.exec() != QDialog::Accepted)
    {
        int index = box->findData(m_paramDefine.dataType);
        box->setCurrentIndex(index);
        return;
    }

    int new_type = dialog.dataType();
    if (m_paramDefine.dataType != new_type)
    {        
        TypeEditHelp help;
        //help.init(new_type);
        //help.update(ui->comboBox);

        ui->lineValue->clear();
        if (JZNodeType::isEnum(new_type))
        {
            ui->boxValue->show();
            ui->lineValue->hide();

            UiHelper::updateEnumBox(ui->boxValue,new_type);            
        }
        else
        {
            ui->boxValue->hide();
            ui->lineValue->show();
            ui->lineValue->setEnabled(JZNodeType::isBase(new_type));
        }
    }
*/
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