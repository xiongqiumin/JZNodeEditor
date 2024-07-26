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
#include "JZNodeQtBind.h"

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

void JZNodeParamBindEditDialog::init(JZScriptClassItem *item, QString variable)
{
    ui->boxWidget->clear();

    m_bind.variable = variable;    
    m_bind.dir = JZNodeParamBind::UiToData;

    auto var = item->memberVariable(variable, false);

    ui->boxWidget->addItem("不绑定");
    auto list = item->memberVariableList(true);
    for (int i = 0; i < list.size(); i++)
    { 
        auto def = item->memberVariable(list[i],true);
        if (!JZNodeType::isInherits(def->type, "QWidget"))
            continue;

        /*
        auto info = JZNodeQtBind::BindSupport(def->type);               
        if (info.dataType.contains(var->type))
            ui->boxWidget->addItem(list[i]);
        */
    }
}

void JZNodeParamBindEditDialog::setParamBind(JZNodeParamBind bind)
{
    m_bind = bind;
    ui->boxWidget->setCurrentText(bind.widget);
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
    if (ui->boxWidget->currentIndex() == 0)
        m_bind.widget.clear();
    else
        m_bind.widget = ui->boxWidget->currentText();

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