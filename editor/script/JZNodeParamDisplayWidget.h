#ifndef JZNODE_PARAM_DISPLAY_WIDGET_H_
#define JZNODE_PARAM_DISPLAY_WIDGET_H_

#include <QLabel>
#include <QToolButton>
#include "JZNodePinWidget.h"
#include "QImageLabel.h"

//JZNodeParamDisplayWidget
class JZCORE_EXPORT JZNodeParamDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamDisplayWidget();
    virtual ~JZNodeParamDisplayWidget();

    virtual void setRuntimeValue(const JZNodeDebugParamValue &value) = 0;
};

//JZNodeImageDisplayWidget
class JZNodeImageDisplayWidget : public JZNodeParamDisplayWidget
{
    Q_OBJECT

public:
    JZNodeImageDisplayWidget();
    virtual ~JZNodeImageDisplayWidget();

    virtual void setRuntimeValue(const JZNodeDebugParamValue &value) override;    

protected:
    QImageLabel *m_label;
};


#endif