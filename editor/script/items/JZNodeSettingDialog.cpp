#include "JZNodeSettingDialog.h"
#include <QDebug>

JZNodeSettingDialog::JZNodeSettingDialog(const QJsonObject& json, QWidget *parent) : QDialog(parent), originalJson(json)
{    
}

JZNodeSettingDialog::~JZNodeSettingDialog()
{
}