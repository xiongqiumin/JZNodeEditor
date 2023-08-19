#pragma once

#include <QDialog>
#include <QTableWidget>
#include "JZNode.h"
#include "JZScriptFile.h"

namespace Ui { class JZNodeParamEditDialog; }

class JZNodeParamEditDialog : public QDialog
{
    Q_OBJECT

public:
    JZNodeParamEditDialog(QWidget *parent = Q_NULLPTR);
    ~JZNodeParamEditDialog();
        
    void init(JZScriptFile *file,JZParamDefine info);
    JZParamDefine param();

protected slots:      
    void onTypeChanged(int index);
    void on_btnOk_clicked();
    void on_btnCancel_clicked();    

private:    
    void uiToData();
    void dataToUi();

    JZParamDefine m_paramDefine;
    QString m_preName;
    JZScriptFile *m_file;    
    Ui::JZNodeParamEditDialog *ui;
};
