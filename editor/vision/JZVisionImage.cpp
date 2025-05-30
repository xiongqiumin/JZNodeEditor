#include <QHBoxLayout>
#include <QVBoxLayout>
#include "JZVisionImage.h"

JZVisionImage::JZVisionImage(QWidget *parent)
    : QWidget(parent)
{
    m_view = new JZImageView();

    QWidget* top = new QWidget();
    QHBoxLayout* l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* v = new QVBoxLayout();
    top->setLayout(l);
    v->addWidget(top);
    v->addWidget(m_view);
    setLayout(v);
}

JZVisionImage::~JZVisionImage()
{
}

JZImageView* JZVisionImage::view()
{
    return m_view;
}