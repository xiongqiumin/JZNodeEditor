#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QThread>
#include "JZUpdateDialog.h"
#include "JZUpdateClient.h"


class JZUpdateClientThread : public QThread
{
public:
    QThread *preThread;
    JZUpdateClient *client;

    virtual void run() override
    {
        client->downloadUpdate();
        client->moveToThread(preThread);
    }
};

//
JZUpdateDialog::JZUpdateDialog(QWidget *parent)
    :QDialog(parent)
{
    m_client = nullptr;
    QHBoxLayout *h = new QHBoxLayout();
    QVBoxLayout *l = new QVBoxLayout();    

    m_btn = new QPushButton("cancel");
    connect(m_btn, &QPushButton::clicked, this, &JZUpdateDialog::onBtnCancel);

    m_bar = new QProgressBar();
    m_label = new QLabel("wating...");    
    m_bar->setRange(0, 100);
    m_bar->setValue(0);    

    h->addStretch();
    h->addWidget(m_btn);
    h->setContentsMargins(0, 0, 0, 0);
    
    l->addWidget(m_label);
    l->addWidget(m_bar);    
    l->addLayout(h);
    setLayout(l);

    setFixedSize(300, 120);
}

JZUpdateDialog::~JZUpdateDialog()
{
}

void JZUpdateDialog::setClient(JZUpdateClient *client)
{
    m_client = client;
    connect(m_client, &JZUpdateClient::sigProgess, this, &JZUpdateDialog::onUpdateProcess);
    connect(m_client, &JZUpdateClient::sigError, this, &JZUpdateDialog::onUpdateError);
}

void JZUpdateDialog::onBtnCancel()
{
    reject();    
}

void JZUpdateDialog::onUpdateError(QString label)
{
    m_label->setText(label);
}

void JZUpdateDialog::onUpdateProcess(int step, QString label)
{
    m_bar->setValue(step);
    m_label->setText(label);
    
    qApp->processEvents();    
}

int JZUpdateDialog::exec()
{    
    JZUpdateClientThread t;
    t.client = m_client;
    t.preThread = QThread::currentThread();
    m_cliendThread = &t;
    m_client->moveToThread(&t);
    t.start();
    connect(&t, &QThread::finished, this, [this]
    {        
        if (m_client->isDownloadFinish())
            this->accept();
    });
    return QDialog::exec();    
}

void JZUpdateDialog::reject()
{
    if(m_client->isDownload())
        m_client->cancel();
    m_cliendThread->wait();

    QDialog::reject();
}