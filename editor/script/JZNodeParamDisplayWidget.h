#ifndef JZNODE_PARAM_DISPLAY_WIDGET_H_
#define JZNODE_PARAM_DISPLAY_WIDGET_H_

#include <QLabel>
#include <QToolButton>
#include "JZNodePinWidget.h"
#include "jzWidgets/JZImageLabel.h"

//JZNodeParamDisplayWidget
class JZNodeParamDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamDisplayWidget();
    virtual ~JZNodeParamDisplayWidget();

    virtual void setRuntimeValue(const JZNodeDebugParamValue &value) = 0;
};

//JZNodeImageNomarlDisplayWidget
class JZNodeImageNomarlDisplayWidget : public JZNodeParamDisplayWidget
{
    Q_OBJECT

public:
    JZNodeImageNomarlDisplayWidget();
    virtual ~JZNodeImageNomarlDisplayWidget();

    virtual void setRuntimeValue(const JZNodeDebugParamValue &value) override;    

protected:
    QLineEdit *m_lineEdit;
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
    JZImageLabel *m_label;
};


#endif