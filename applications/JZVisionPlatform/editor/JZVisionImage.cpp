#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include "JZVisionImage.h"
#include "modules/opencv/CvToQt.h"

JZVisionImage::JZVisionImage(QWidget *parent)
    : QWidget(parent)
{
    m_view = new JZImageView();
    connect(m_view, &JZImageView::sigCoorColor, this, &JZVisionImage::onCoorColor);

    QWidget* top = new QWidget();
    QHBoxLayout* top_l = new QHBoxLayout();
    top_l->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* v = new QVBoxLayout();
    v->setContentsMargins(0, 0, 0, 0);
    top->setLayout(top_l);

    m_imageBox = new QComboBox();
    connect(m_imageBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &JZVisionImage::onImageBoxChanged);
    top_l->addWidget(m_imageBox);
    top_l->addStretch();

    QToolButton *btnZoomIn = new QToolButton();
    QToolButton *btnZoomOut = new QToolButton();
    QToolButton *btnFit = new QToolButton();
    top_l->addWidget(btnZoomIn);
    top_l->addWidget(btnZoomOut);
    top_l->addWidget(btnFit);
    connect(btnZoomIn, &QToolButton::clicked, this, &JZVisionImage::onBtnZoomIn);
    connect(btnZoomOut, &QToolButton::clicked, this, &JZVisionImage::onBtnZoomOut);
    connect(btnFit, &QToolButton::clicked, this, &JZVisionImage::onBtnFit);

    QWidget* bottom = new QWidget();
    QHBoxLayout* bottom_l = new QHBoxLayout(bottom);
    bottom_l->setContentsMargins(0, 0, 0, 0);

    m_status = new QLabel();
    bottom_l->addWidget(m_status);

    v->addWidget(top);
    v->addWidget(m_view);
    v->addWidget(bottom);
    setLayout(v);
}

JZVisionImage::~JZVisionImage()
{
}

void JZVisionImage::onImageBoxChanged(int index)
{

}

void JZVisionImage::onBtnZoomIn()
{
    m_view->scale(1.05,1.05);
}

void JZVisionImage::onBtnZoomOut()
{
    m_view->scale(0.95, 0.95);
}

void JZVisionImage::onBtnFit()
{
    QSize size = m_view->image().size();
    m_view->fitInView(QRectF(0, 0, size.width(), size.height()), Qt::KeepAspectRatio);
}

void JZVisionImage::onCoorColor(QPoint pos,QColor color)
{
    QSize size = m_view->image().size();
    QString size_str = QString::asprintf("%d * %d | ", size.width(), size.height());
    QString coor_info = QString::asprintf("X,%d Y,%d | R:%d G:%d B:%d",pos.x(),pos.y(),color.red(),color.green(),color.blue()); 
    m_status->setText(size_str + coor_info);
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

void JZVisionImage::setImage(const QList<ImageResult> &outputImage)
{
    m_view->setImage(QtOcv::mat2Image(outputImage[0].mat));
    m_view->initGraphics(outputImage[0].graphList);
}