#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include "JZBaseDialog.h"

//JZBaseDialog
JZBaseDialog::JZBaseDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *verticalLayout = new QVBoxLayout();

    m_mainWidget = new QWidget();
    
    m_btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    
    connect(m_btnBox, &QDialogButtonBox::accepted, this, &JZBaseDialog::accept);
    connect(m_btnBox, &QDialogButtonBox::rejected, this, &JZBaseDialog::reject);

    this->setLayout(verticalLayout);
    verticalLayout->addWidget(m_mainWidget);
    verticalLayout->addWidget(m_btnBox);
}

JZBaseDialog::~JZBaseDialog()
{
}

void JZBaseDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {

    }
    else
        QDialog::keyPressEvent(event);
}

void JZBaseDialog::setCentralWidget(QWidget *w)
{
    delete m_mainWidget;
    QVBoxLayout *l = qobject_cast<QVBoxLayout*>(layout());
    l->insertWidget(0, w);
    m_mainWidget = w;
}