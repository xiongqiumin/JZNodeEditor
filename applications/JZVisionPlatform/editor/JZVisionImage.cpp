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
    int node_id = m_imageBox->itemData(index).toInt();
    if (m_imageResult.contains(node_id))
    {
        setResult(m_imageResult[node_id]);
    }
    else
    {
        m_view->clear();
        m_status->setText(QString());
    }
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
        auto node_info = node_list[i];
        auto node = node_list[i].node;
        if(node_info.hasImage)
            m_imageBox->addItem(node_list[i].name, node->id());
    }
}

void JZVisionImage::setResult(const ImageResult &result)
{
    m_view->clear();
    m_view->setImage(QtOcv::mat2Image(result.mat));
    m_view->initGraphics(result.graphList);
}

void JZVisionImage::clear()
{
    m_imageResult.clear();
    m_view->clear();
}

void JZVisionImage::setImage(int node_id, const ImageResult &outputImage)
{
    m_imageResult[node_id] = outputImage;
    if (m_imageBox->currentIndex() != -1 && m_imageBox->currentData().toInt() == node_id)
        setResult(outputImage);
}