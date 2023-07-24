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

protected:
    Ui::JZNewClassDialog *ui;
};





















#endif
