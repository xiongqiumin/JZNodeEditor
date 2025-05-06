#include <QHBoxLayout>
#include "JZNodeParamDisplayWidget.h"
#include "mainwindow.h"

//JZNodeParamDisplayWidget
JZNodeParamDisplayWidget::JZNodeParamDisplayWidget()
{    
}

JZNodeParamDisplayWidget::~JZNodeParamDisplayWidget()
{

}

//JZNodeImageDisplayWidget
JZNodeImageDisplayWidget::JZNodeImageDisplayWidget()
{    
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);    
    m_label = new JZImageLabel();
    l->addWidget(m_label);
    setLayout(l);

    resize(160, 160);
}

JZNodeImageDisplayWidget::~JZNodeImageDisplayWidget()
{    
}
