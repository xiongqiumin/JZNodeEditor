#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include "JZNodeValue.h"
#include "JZNodePinWidget.h"

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
    void onBtnOkClicked();
    void onBtnCancelClicked();
    void onTypeChanged();

private:
    QLineEdit *m_lineName;
    JZNodeParamTypeWidget *m_typeWidget;
    JZNodePinValueWidget *m_valueWidget;
    JZParamDefine m_define;
};
