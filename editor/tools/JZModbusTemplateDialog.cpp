#include <QHBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QStackedWidget>
#include <QCheckBox>
#include "JZModbusTemplateDialog.h"
#include "JZSearchTreeWidget.h"

//ModbusStargeDialog
JZModbusTemplateDialog::JZModbusTemplateDialog(QWidget *widget)
    :JZBaseDialog(widget)
{
    QFormLayout *l = new QFormLayout();
    l->setContentsMargins(0, 0, 0, 0);

    JZSearchTreeWidget *tree = new JZSearchTreeWidget();
    l->addWidget(tree);

    m_tree = tree->tree();
    m_mainWidget->setLayout(l);
}

JZModbusConfig JZModbusTemplateDialog::config()
{
    return m_config;
}

bool JZModbusTemplateDialog::onOk()
{
    return true;
}

void JZModbusTemplateDialog::initConfig()
{

}