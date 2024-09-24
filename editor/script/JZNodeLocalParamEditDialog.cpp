#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include "JZNodeLocalParamEditDialog.h"

//JZNodeLocalParamEditDialog
JZNodeLocalParamEditDialog::JZNodeLocalParamEditDialog(QWidget *parent)
    : QDialog(parent)
{    
    QVBoxLayout *verticalLayout = new QVBoxLayout();
    
    QWidget *widget_grid = new QWidget();    
    QGridLayout *gridLayout = new QGridLayout(widget_grid);    
    gridLayout->setContentsMargins(0, 0, 0, 0);

    m_lineName = new QLineEdit();
    gridLayout->addWidget(new QLabel("参数名:"), 0, 0);    
    gridLayout->addWidget(m_lineName, 0, 1);
    
    m_typeWidget = new JZNodeParamTypeWidget();
    gridLayout->addWidget(new QLabel("类型:"), 1, 0);    
    gridLayout->addWidget(m_typeWidget, 1, 1);
    connect(m_typeWidget,SIGNAL(sigTypeChanged()),this, SLOT(onTypeChanged()));

    m_valueWidget = new JZNodePinValueWidget();
    gridLayout->addWidget(new QLabel("默认值:"), 2, 0);    
    gridLayout->addWidget(m_valueWidget, 2, 1);    

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *btnOk = new QPushButton("Ok");    
    QPushButton *btnCancel = new QPushButton("Cancel");
    connect(btnOk, &QPushButton::clicked, this, &JZNodeLocalParamEditDialog::onBtnOkClicked);
    connect(btnCancel, &QPushButton::clicked, this, &JZNodeLocalParamEditDialog::onBtnCancelClicked);

    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    this->setLayout(verticalLayout);
    verticalLayout->addWidget(widget_grid);
    verticalLayout->addLayout(btnLayout);    

    setParam(JZParamDefine());
}

JZNodeLocalParamEditDialog::~JZNodeLocalParamEditDialog()
{
}

void JZNodeLocalParamEditDialog::setParam(JZParamDefine define)
{    
    auto env = editorEnvironment();

    m_lineName->setText(define.name);
    m_typeWidget->setType(define.type);    
    m_valueWidget->initWidget(env->nameToType(define.type));
    m_valueWidget->setValue(define.value);
}

JZParamDefine JZNodeLocalParamEditDialog::param()
{    
    return m_define;
}

void JZNodeLocalParamEditDialog::onTypeChanged()
{
    int data_type = editorEnvironment()->nameToType(m_typeWidget->type());
    m_valueWidget->initWidget(data_type);
}

void JZNodeLocalParamEditDialog::onBtnOkClicked()
{    
    m_define.name = m_lineName->text();
    m_define.type = m_typeWidget->type();
    m_define.value = m_valueWidget->value();
    QDialog::accept();
}

void JZNodeLocalParamEditDialog::onBtnCancelClicked()
{
    QDialog::reject();
}