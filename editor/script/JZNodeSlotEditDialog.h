#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QListWidget>
#include "JZNodeFunction.h"
#include "JZBaseDialog.h"
#include "JZClassItem.h"

class JZNodeSlotEditDialog : public JZBaseDialog
{
    Q_OBJECT

public:
    JZNodeSlotEditDialog(QWidget *parent = Q_NULLPTR);
    ~JZNodeSlotEditDialog();

    void setClass(JZScriptClassItem *cls);    
    QString param();
    QString signal();

protected slots:
    void onListParamChanged(QListWidgetItem *current);

protected:    
    virtual bool onOk();

    QListWidget *m_listParam;
    QListWidget *m_listSingle;
    JZNodeObjectDefine m_class;
    QString m_param;
    QString m_signal;
    
};
