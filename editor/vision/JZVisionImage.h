#pragma once

#include <QWidget>
#include "jzWidgets/JZImageView.h"

class JZVisionImage : public QWidget
{
    Q_OBJECT
    
public:
    explicit JZVisionImage(QWidget *parent = nullptr);
    ~JZVisionImage();

    JZImageView* view();

protected:
    JZImageView* m_view;
};