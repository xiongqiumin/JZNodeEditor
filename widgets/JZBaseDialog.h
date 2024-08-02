#ifndef JZBASE_DIALOG_H_
#define JZBASE_DIALOG_H_

#include <QDialog>
#include "UiCommon.h"

class JZBaseDialog : public QDialog
{
    Q_OBJECT

public:
    JZBaseDialog(QWidget *parent = Q_NULLPTR);
    ~JZBaseDialog();       

protected slots:
    void onBtnOkClicked();
    void onBtnCancelClicked();    

protected:
    enum {
        Button_Ok,
        Button_Cancel,
    };

	virtual bool onOk() = 0;
	virtual bool onCancel();
    void setCentralWidget(QWidget *w);
    void showButton(int btn, bool show);    

    QWidget *m_mainWidget;	
    QList<QPushButton*> m_buttons;
};

#endif
