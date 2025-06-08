#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include "jzWidgets/JZImageView.h"
#include "../JZVisionNodeInfo.h"

class JZVisionImage : public QWidget
{
    Q_OBJECT
    
public:
    explicit JZVisionImage(QWidget *parent = nullptr);
    ~JZVisionImage();

    JZImageView* view();
    void clear();

    void initNodeList(const QList<JZVisionNodeInfo> &node_list);
    void setImage(const QList<ImageResult> &image);

signals:
    void sigImageChanged(int node);

protected slots:
    void onImageBoxChanged(int index);    
    void onBtnZoomIn();
    void onBtnZoomOut();
    void onBtnFit();

    void onCoorColor(QPoint pos,QColor color);

protected:
    QComboBox *m_imageBox;
    JZImageView* m_view;
    QLabel *m_status;
};