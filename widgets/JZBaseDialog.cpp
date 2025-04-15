#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include "JZBaseDialog.h"

//JZBaseDialog
JZBaseDialog::JZBaseDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *verticalLayout = new QVBoxLayout();

    m_mainWidget = new QWidget();
    
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    
    connect(box, &QDialogButtonBox::accepted, this, &JZBaseDialog::onBtnOkClicked);
    connect(box, &QDialogButtonBox::rejected, this, &JZBaseDialog::onBtnCancelClicked);

    this->setLayout(verticalLayout);
    verticalLayout->addWidget(m_mainWidget);
    verticalLayout->addWidget(box);
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