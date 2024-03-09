#ifndef JZBASE_DIALOG_H_
#define JZBASE_DIALOG_H_

#include <QDialog>


class JZBaseDialog : public QDialog
{
    Q_OBJECT

public:
    JZBaseDialog(QWidget *parent = Q_NULLPTR);
    ~JZBaseDialog();       

protected slots:
    void onBtnOkClicked();
    void onBtnCancelClicked();    

private:    	    
	virtual bool onOk();
	virtual bool onCancel();

    QWidget *m_mainWidget;	
};

#endif
