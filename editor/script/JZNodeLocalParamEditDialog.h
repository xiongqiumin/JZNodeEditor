#pragma once

#include <QDialog>
#include <QTableWidget>
#include "JZNodeValue.h"

namespace Ui { class JZNodeLocalParamEditDialog; }

class QTreeWidgetItem;
class JZNodeLocalParamEditDialog : public QDialog
{
    Q_OBJECT

public:
    JZNodeLocalParamEditDialog(QWidget *parent = Q_NULLPTR);
    ~JZNodeLocalParamEditDialog();

    void setParam(JZParamDefine define);
    JZParamDefine param();

protected slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

private:
    Ui::JZNodeLocalParamEditDialog *ui;
};
