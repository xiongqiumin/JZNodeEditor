#include <QMessageBox>
#include "JZNodeModuleDialog.h"
#include "JZModule.h"

//JZNodeModuleDialog
JZNodeModuleDialog::JZNodeModuleDialog(QWidget *parent)
    : QDialog(parent)
{
}

JZNodeModuleDialog::~JZNodeModuleDialog()
{
}

void JZNodeModuleDialog::init()
{
    auto list = JZModuleManager::instance()->moduleList();
    for(int i = 0; i < list.size(); i++)
    {
        auto info = JZModuleManager::instance()->moduleInfo(list[i]);
    }
}