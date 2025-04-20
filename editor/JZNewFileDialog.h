#ifndef JZNODE_NEW_FILE_DIALOG_H_
#define JZNODE_NEW_FILE_DIALOG_H_

#include <QDialog>

namespace Ui { class JZNewFileDialog; }

class JZNewFileDialog : public QDialog
{
    Q_OBJECT
    
public:
    enum {
        NewFile,
        NewClass,
        NewUiClass,
    };

    JZNewFileDialog(QWidget *p = nullptr);
    ~JZNewFileDialog();

    void init(QString path);

    QString name();
    QString path();
    int type();

protected slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

protected:    
    Ui::JZNewFileDialog *ui;
};





















#endif
