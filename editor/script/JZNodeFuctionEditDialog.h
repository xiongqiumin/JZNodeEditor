#pragma once

#include <QDialog>
#include <QListWidget>
#include "JZNodeFunction.h"

namespace Ui { class JZNodeFuctionEditDialog; }

class JZNodeFuctionEditDialog : public QDialog
{
    Q_OBJECT

public:
    JZNodeFuctionEditDialog(QWidget *parent = Q_NULLPTR);
    ~JZNodeFuctionEditDialog();

    void setFunctionInfo(FunctionDefine info);
    FunctionDefine functionInfo();    

protected slots:    
    

private:
    FunctionDefine m_functionDefine;

    Ui::JZNodeFuctionEditDialog *ui;
};
