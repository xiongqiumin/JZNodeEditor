#ifndef JZNODE_PARAM_DISPLAY_WIDGET_H_
#define JZNODE_PARAM_DISPLAY_WIDGET_H_

#include <QLabel>
#include <QToolButton>
#include "jzWidgets/JZImageLabel.h"

//JZNodeParamDisplayWidget
class JZNodeParamDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamDisplayWidget();
    virtual ~JZNodeParamDisplayWidget();    
};

//JZNodeImageDisplayWidget
class JZNodeImageDisplayWidget : public JZNodeParamDisplayWidget
{
    Q_OBJECT

public:
    JZNodeImageDisplayWidget();
    virtual ~JZNodeImageDisplayWidget();

protected:
    JZImageLabel *m_label;
};


#endif