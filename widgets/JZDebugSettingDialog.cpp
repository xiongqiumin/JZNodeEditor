#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include "JZDebugSettingDialog.h"

//JZDebugSetting
JZDebugSetting::JZDebugSetting()
{
    type = Local;
    ip = "192.168.0.105";
    port = 15888;
}

//JZDebugSettingDialog
JZDebugSettingDialog::JZDebugSettingDialog(QWidget *parent)
    :JZBaseDialog(parent)
{
    ui.setupUi(m_mainWidget);
}

void JZDebugSettingDialog::setConfig(JZDebugSetting config)
{
    if (config.type == JZDebugSetting::Local)
        ui.radioLocal->setChecked(true);
    else
        ui.radioRemote->setChecked(true);

    ui.lineRemoteIp->setText(config.ip);
    ui.lineRemotePort->setText(QString::number(config.port));
}

JZDebugSetting JZDebugSettingDialog::config()
{
    JZDebugSetting config;
    if (ui.radioLocal->isChecked())
        config.type = JZDebugSetting::Local;
    else
        config.type = JZDebugSetting::Remote;

    config.ip = ui.lineRemoteIp->text();
    config.port = ui.lineRemotePort->text().toInt();
    return config;
}