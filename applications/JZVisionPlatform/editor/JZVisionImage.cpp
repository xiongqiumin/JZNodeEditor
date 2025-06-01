#include <QHBoxLayout>
#include <QVBoxLayout>
#include "JZVisionImage.h"

JZVisionImage::JZVisionImage(QWidget *parent)
    : QWidget(parent)
{
    m_view = new JZImageView();

    QWidget* top = new QWidget();
    QHBoxLayout* top_l = new QHBoxLayout();
    top_l->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* v = new QVBoxLayout();
    v->setContentsMargins(0, 0, 0, 0);
    top->setLayout(top_l);

    m_imageBox = new QComboBox();
    connect(m_imageBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &JZVisionImage::onImageBoxChanged);
    top_l->addWidget(m_imageBox);

    v->addWidget(top);
    v->addWidget(m_view);
    setLayout(v);
}

JZVisionImage::~JZVisionImage()
{
}

void JZVisionImage::onImageBoxChanged(int index)
{

}

JZImageView* JZVisionImage::view()
{
    return m_view;
}

void JZVisionImage::initNodeList(const QList<JZVisionNodeInfo> &node_list)
{
    m_imageBox->clear();

    for (int i = 0; i < node_list.size(); i++)
    {
        auto node = node_list[i].node;
        m_imageBox->addItem(node->name(),node->id());
    }
}

void JZVisionImage::clear()
{
    m_view->clear();
}