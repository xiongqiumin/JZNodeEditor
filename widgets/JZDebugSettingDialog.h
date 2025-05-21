#ifndef JZ_DEBUG_SETTING_DIALOG_H_
#define JZ_DEBUG_SETTING_DIALOG_H_

#include "JZBaseDialog.h"
#include "ui_JZDebugSettingDialog.h"

namespace Ui { class JZDebugSettingDialog; }

//JZDebugSetting
class JZDebugSetting
{
public:
    enum {
        Local,
        Remote,
    };

    JZDebugSetting();

    int type;
    QString ip;
    int port;
};

//JZDebugSettingDialog
class JZDebugSettingDialog: public JZBaseDialog
{
public:
    JZDebugSettingDialog(QWidget *parent);

    void setConfig(JZDebugSetting config);
    JZDebugSetting config();

protected:
    Ui::JZDebugSettingDialog ui;
};


#endif // !JZ_ABOUT_DIALOG_H_
