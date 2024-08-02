#ifndef JZ_UPDATE_DIALOG_H
#define JZ_UPDATE_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

class JZUpdateClient;
class JZUpdateDialog : public QDialog
{
	Q_OBJECT

public:
    JZUpdateDialog(QWidget *parent = nullptr);
	~JZUpdateDialog();
	
    void setClient(JZUpdateClient *client);
    virtual int exec() override;
    virtual void reject() override;


protected slots:
    void onBtnCancel();    
    void onUpdateProcess(int step,QString label);
    void onUpdateError(QString label);

private:                
    JZUpdateClient *m_client;
    QPushButton *m_btn;
    QProgressBar *m_bar;
    QLabel *m_label;
    QThread *m_cliendThread;
};

#endif // UPDATE_H
