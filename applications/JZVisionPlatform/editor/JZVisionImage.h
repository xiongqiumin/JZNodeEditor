#pragma once

#include <QWidget>
#include <QComboBox>
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

signals:
    void sigImageChanged(int node);

protected slots:
    void onImageBoxChanged(int index);

protected:
    QComboBox *m_imageBox;
    JZImageView* m_view;
};