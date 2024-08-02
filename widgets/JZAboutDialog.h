#ifndef JZ_ABOUT_DIALOG_H_
#define JZ_ABOUT_DIALOG_H_

#include "JZBaseDialog.h"

class JZAboutDialog: public JZBaseDialog
{
public:
    JZAboutDialog(QWidget *parent);

protected:
    virtual bool onOk() override;
};


#endif // !JZ_ABOUT_DIALOG_H_
