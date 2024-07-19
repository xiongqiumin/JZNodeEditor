#ifndef HHNEWPROJECTDLG_H
#define HHNEWPROJECTDLG_H

#include <QDialog>
#include <QTextEdit>
#include <QStackedWidget>
#include "UiCommon.h"
#include "JZProject.h"
#include "JZBaseDialog.h"

class JZProjectSettingDialog : public JZBaseDialog
{
	Q_OBJECT

public:
	JZProjectSettingDialog(QWidget *parent = 0);
	~JZProjectSettingDialog();

	void setProject(JZProject *project);

protected slots:
	void onBtnNagtiveClicked();

protected:
	virtual bool onOk();

	QWidget *addTitle(QWidget *w,QString title,QString help);

	QStackedWidget *m_stackWidget;
	QTextEdit *m_moduleEdit;
	QTextEdit *m_containerEdit;
    JZProject *m_project;
};

#endif // HHNEWPROJECTDLG_H
