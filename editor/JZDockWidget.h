#ifndef JZ_DOCK_WIDGET_H_
#define JZ_DOCK_WIDGET_H_

#include <QWidget>

class JZDockWidget : public QWidget
{
public:    
    JZDockWidget(QString title,QWidget *subWidget);
    ~JZDockWidget();
    
};

#endif