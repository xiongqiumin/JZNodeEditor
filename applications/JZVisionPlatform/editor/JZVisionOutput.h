#pragma once
#include <QWidget>

class JZVisionOutput : public QWidget
{
    Q_OBJECT
    
public:
    explicit JZVisionOutput(QWidget *parent = nullptr);
    ~JZVisionOutput();
};