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
    auto env = editorEnvironment();
    auto d = env->editorManager()->delegate(Type_image);
    QVariant v = d->unpack(env,value.binValue);

    auto image = editorObjectManager()->objectCast<QImage>(v);
    m_label->setImage(*image);
}