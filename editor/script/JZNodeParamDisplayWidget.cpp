#include <QHBoxLayout>
#include "JZNodeParamDisplayWidget.h"
#include "JZNodeParamDelegate.h"
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
    m_label = new QImageLabel();
    l->addWidget(m_label);
    setLayout(l);

    resize(160, 160);
}

JZNodeImageDisplayWidget::~JZNodeImageDisplayWidget()
{    
}

void JZNodeImageDisplayWidget::setRuntimeValue(const JZNodeDebugParamValue &value)
{
    auto d = JZNodeParamDelegateManager::instance()->delegate(Type_image);
    QVariant v = d->unpack(value.binValue);
    auto env = g_mainWindow->project()->environment();
    auto image = env->objectCast<QImage>(v);
    m_label->setImage(*image);
}