#include <QHBoxLayout>
#include <QFileDialog>
#include <QIcon>
#include "JZNodeEditorWidget.h"

JZNodeImageEditWidget::JZNodeImageEditWidget()
{
    m_pixmapLabel = new QLabel(this);
    m_pathLabel = new QLabel(this);
    m_button = new QToolButton(this);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(m_pixmapLabel);
    layout->addWidget(m_pathLabel);
    layout->setMargin(0);
    layout->setSpacing(0);
    m_pixmapLabel->setFixedWidth(16);
    m_pixmapLabel->setAlignment(Qt::AlignCenter);
    m_pathLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));

    m_button->setText(tr("..."));
    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(30);
    layout->addWidget(m_button);    

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));    
    connect(m_button, &QAbstractButton::clicked, this, &JZNodeImageEditWidget::onFileActionActivated);
}

JZNodeImageEditWidget::~JZNodeImageEditWidget()
{

}
    
QString JZNodeImageEditWidget::value()
{
    return m_path;
}

void JZNodeImageEditWidget::setValue(const QString &text)
{    
    m_path = text;

    if (m_path.isEmpty()) {
        m_pathLabel->setText(m_path);
        m_pixmapLabel->setPixmap(QPixmap());        
    }
    else {
        QPixmap pixmap(m_path);
        pixmap = pixmap.scaled(16, 16, Qt::KeepAspectRatio);

        m_pathLabel->setText(QFileInfo(m_path).fileName());
        m_pixmapLabel->setPixmap(pixmap);
    }
}

void JZNodeImageEditWidget::onFileActionActivated()
{
    const QString newPath = QFileDialog::getOpenFileName(this, m_path, "", "*.bmp;*.jpg;*.png");
    if (!newPath.isEmpty() && newPath != m_path) {        
        setValue(newPath);
        emit sigValueChanged();
    }
}

QVariant createImage(const QString &value)
{
    QImage *image = new QImage(value);
    return JZObjectCreateRefrence(image,true);
}

void InitEditorWidget()
{
    JZNodeParamWidgetManager::instance()->registEditWidget(Type_imageEdit, CreateParamEditWidget<JZNodeImageEditWidget>);
    JZNodeParamWidgetManager::instance()->registEditDelegate(Type_image, Type_imageEdit, createImage);
}