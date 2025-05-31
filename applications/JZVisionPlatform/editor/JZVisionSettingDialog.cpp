#include <QTabWidget>
#include "JZVisionSettingDialog.h"

JZVisionSettingDialog::JZVisionSettingDialog(QWidget *parent) 
    : JZBaseDialog(parent)
{
    JZPropertyBrowser *p = new JZPropertyBrowser();

    QTabWidget *tab = new QTabWidget();
    tab->addTab(p,"基本参数");
    setCentralWidget(tab);

    resize(480, 600);
}

JZVisionSettingDialog::~JZVisionSettingDialog()
{
}

void JZVisionSettingDialog::setNode(JZNode* node)
{

}