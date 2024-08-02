#ifndef JZ_NEW_PROJECT_DIALOG_H_
#define JZ_NEW_PROJECT_DIALOG_H_

#include <QDialog>
#include "UiCommon.h"

namespace Ui {class JZNewProjectDialog;}

class JZNewProjectDialog : public QDialog
{
	Q_OBJECT

public:
	JZNewProjectDialog(QWidget *parent = 0);
	~JZNewProjectDialog();

    int projectType();
	QString name();
	QString dir();

protected slots:
    void on_btnSelect_clicked();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

protected:
    QString m_title;

	Ui::JZNewProjectDialog *ui;
    
};

#endif // HHNEWPROJECTDLG_H
