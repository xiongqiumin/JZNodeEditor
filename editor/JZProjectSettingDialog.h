#ifndef JZ_PROJECT_DIALOG_H
#define JZ_PROJECT_DIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QStackedWidget>
#include "UiCommon.h"
#include "JZProject.h"
#include "JZBaseDialog.h"
#include <QTreeWidget>

class JZProjectSettingDialog : public JZBaseDialog
{
	Q_OBJECT

public:
	JZProjectSettingDialog(QWidget *parent = 0);
	~JZProjectSettingDialog();

	void setProject(JZProject *project);

protected slots:
    void onTreeItemClicked(QTreeWidgetItem *current, int col);

protected:
	virtual bool onOk();

	QWidget *addPage(QWidget *w,QString help);

	QStackedWidget *m_stackWidget;
	QTextEdit *m_moduleEdit;
	QTextEdit *m_containerEdit;
    JZProject *m_project;
    QTreeWidget *m_tree;
};

#endif
