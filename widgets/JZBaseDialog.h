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

protected:
    enum {
        Button_Ok,
        Button_Cancel,
    };	
    virtual void keyPressEvent(QKeyEvent *event) override;
    void setCentralWidget(QWidget *w);
    void showButton(int btn, bool show);        

    QWidget *m_mainWidget;	
    QList<QPushButton*> m_buttons;
};

#endif
