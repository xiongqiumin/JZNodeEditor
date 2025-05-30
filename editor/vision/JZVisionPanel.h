#pragma once
#include <QWidget>

class JZVisionPanel : public QWidget
{
    Q_OBJECT
    
public:
    explicit JZVisionPanel(QWidget *parent = nullptr);
    ~JZVisionPanel();
};