#include <QLabel>
#include <QVBoxLayout>
#include "JZDockWidget.h"

JZDockWidget::JZDockWidget(QString title,QWidget *subWidget)
{    
    QLabel *list_title = new QLabel(" " + title);
    list_title->setStyleSheet("background-color: rgb(77,96,130); color: rgb(255,255,255);");
    list_title->setFixedHeight(30);

    QVBoxLayout *panel_layout = new QVBoxLayout();
    
    panel_layout->setContentsMargins(0, 0, 0, 0);
    panel_layout->setSpacing(0);
    panel_layout->addWidget(list_title);
    panel_layout->addWidget(subWidget);
    setLayout(panel_layout);
}

JZDockWidget::~JZDockWidget()
{
}