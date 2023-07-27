#ifndef JZNODE_NEW_DIALOG_H_
#define JZNODE_NEW_DIALOG_H_

#include <QDialog>

namespace Ui { class JZNewClassDialog; }

class JZNewClassDialog : public QDialog
{
    Q_OBJECT
    
public:
    JZNewClassDialog(QWidget *p = nullptr);
    ~JZNewClassDialog();

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

    Ui::JZNewClassDialog *ui;
};





















#endif
