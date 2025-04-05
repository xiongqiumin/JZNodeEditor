#ifndef JZNODE_PARAM_WIDGET_H_
#define JZNODE_PARAM_WIDGET_H_

#include <QWidget>
#include <QComboBox>
#include "JZProcess.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeParamEditWidget.h"
#include "JZEditorGlobal.h"

//JZNodePinWidget
class JZNode;
class JZNodePinWidget : public QWidget
{
    Q_OBJECT
        
public:
    JZNodePinWidget(JZNode *node, int pin_id);
    ~JZNodePinWidget();
        
    virtual QString value() const;
    virtual void setValue(const QString &value);
    virtual void updateWidget();

signals:
    void sigValueChanged(const QString &value);
    void sigSizeChanged(QSize size);

protected:
    JZNode *m_node;
    int m_pin;
};

//JZNodePinButtonWidget
class QPushButton;
class JZNodePinButtonWidget : public JZNodePinWidget
{
    Q_OBJECT

public:
    JZNodePinButtonWidget(JZNode *node, int pin_id);
    ~JZNodePinButtonWidget();

    QPushButton *button();

protected:
    QPushButton *m_btn;
};

//JZNodePinValueWidget
class JZNodePinValueWidget : public JZNodePinWidget
{
    Q_OBJECT

public:
    JZNodePinValueWidget(JZNode *node, int pin_id);
    ~JZNodePinValueWidget();

    void initWidget(int data_type, QString widget_type = QString());
    QWidget *focusWidget();

    virtual QString value() const override;
    virtual void setValue(const QString &value) override;

protected slots:
    void onValueChanged();

protected:    
    JZNodeParamValueWidget *m_widget;
};

//JZNodePinDisplayWidget
class JZNodePinDisplayWidget : public JZNodePinWidget
{
    Q_OBJECT

public:
    JZNodePinDisplayWidget(JZNode *node, int pin_id);

    void setRuntimeValue(const JZNodeDebugParamValue &value);
    virtual void updateWidget() override;

protected:   
    void createWidget();
    QWidget *m_widget;
    int m_dataType;
};

#endif // !JZNODE_PARAM_WIDGET_H_
