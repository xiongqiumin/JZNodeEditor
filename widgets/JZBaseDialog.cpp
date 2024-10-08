#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "JZBaseDialog.h"

//JZBaseDialog
JZBaseDialog::JZBaseDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *verticalLayout = new QVBoxLayout();

    m_mainWidget = new QWidget();
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *btnOk = new QPushButton("Ok");
    QPushButton *btnCancel = new QPushButton("Cancel");
    connect(btnOk, &QPushButton::clicked, this, &JZBaseDialog::onBtnOkClicked);
    connect(btnCancel, &QPushButton::clicked, this, &JZBaseDialog::onBtnCancelClicked);
    m_buttons << btnOk << btnCancel;

    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    this->setLayout(verticalLayout);
    verticalLayout->addWidget(m_mainWidget);
    verticalLayout->addLayout(btnLayout);
}

JZBaseDialog::~JZBaseDialog()
{
}

void JZBaseDialog::setCentralWidget(QWidget *w)
{
    delete m_mainWidget;
    QVBoxLayout *l = qobject_cast<QVBoxLayout*>(layout());
    l->insertWidget(0, w);
    m_mainWidget = w;
}

void JZBaseDialog::showButton(int btn, bool show)
{
    m_buttons[btn]->setVisible(show);
}

bool JZBaseDialog::onCancel()
{
    return true;
}

void JZBaseDialog::onBtnOkClicked()
{
    if(onOk())
        QDialog::accept();
}

void JZBaseDialog::onBtnCancelClicked()
{
    if (onCancel())
        QDialog::reject();
}