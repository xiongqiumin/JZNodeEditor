#ifndef JZNODE_NEW_CLASS_EDIT_DIALOG_H_
#define JZNODE_NEW_CLASS_EDIT_DIALOG_H_

#include <QDialog>

namespace Ui { class JZNodeClassEditDialog; }

class JZNodeClassEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    JZNodeClassEditDialog(QWidget *p = nullptr);
    ~JZNodeClassEditDialog();

    QString className();
    QString super();
    bool isUi();

protected slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

protected:
    QString m_className;
    QString m_super;
    bool m_isUi;

    Ui::JZNodeClassEditDialog *ui;
};





















#endif
