#ifndef JZNODE_NEW_CLASS_EDIT_DIALOG_H_
#define JZNODE_NEW_CLASS_EDIT_DIALOG_H_

#include <QDialog>
#include "JZClassItem.h"

namespace Ui { class JZNodeClassEditDialog; }

class JZNodeClassEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    JZNodeClassEditDialog(QWidget *p = nullptr);
    ~JZNodeClassEditDialog();

    void setClass(JZScriptClassItem *class_item);

    QString className();
    QString super();
    QString uiFile();

protected slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

protected:
    Ui::JZNodeClassEditDialog *ui;
};





















#endif
