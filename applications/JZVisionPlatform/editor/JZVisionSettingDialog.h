#pragma once

#include <QDialog>
#include "jzWidgets/JZPropertyBrowser.h"
#include "JZBaseDialog.h"
#include "JZNode.h"

class JZVisionSettingDialog : public JZBaseDialog
{
    Q_OBJECT
    
public:
    explicit JZVisionSettingDialog(QWidget *parent = nullptr);
    ~JZVisionSettingDialog();

    void setNode(JZNode* node);
protected:

};