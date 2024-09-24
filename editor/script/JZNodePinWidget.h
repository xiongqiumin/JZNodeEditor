#ifndef JZNODE_PARAM_WIDGET_H_
#define JZNODE_PARAM_WIDGET_H_

#include <QWidget>
#include <QComboBox>
#include "JZProcess.h"
#include "JZNodeDebugPacket.h"
#include "QImageLabel.h"
#include "JZEditorGlobal.h"

class JZCORE_EXPORT JZNodePinPopupWidget: public QWidget
{
    Q_OBJECT

public:    
    JZNodePinPopupWidget(QWidget *parent = nullptr);
    ~JZNodePinPopupWidget();

    QString value();
    void setValue(QString text);    

signals:
    void sigSettingClicked();

protected:
    QLineEdit *m_line;    
};

class JZCORE_EXPORT JZNodeParamTypeWidget : public QComboBox
{
    Q_OBJECT

public:
    JZNodeParamTypeWidget(QWidget *parent = nullptr);

    void init(JZNodeObjectManager *inst);
    QString type();
    void setType(QString type);

signals:
    void sigTypeChanged();

protected:
};

//JZNodePinWidget
class JZNode;
class JZCORE_EXPORT JZNodePinWidget : public QWidget
{
    Q_OBJECT
        
public:
    JZNodePinWidget(QWidget *parent = nullptr);
    ~JZNodePinWidget();
    
    void init(JZNode *node, int pin_id);
    virtual QString value() const;
    virtual void setValue(const QString &value);

    virtual void updateWidget();

signals:
    void sigValueChanged(const QString &value);
    void sigSizeChanged(QSize size);

protected:
    virtual void init();

    JZNode *m_node;
    int m_pin;
};

//JZNodePinButtonWidget
class QPushButton;
class JZCORE_EXPORT JZNodePinButtonWidget : public JZNodePinWidget
{
    Q_OBJECT

public:
    JZNodePinButtonWidget();
    ~JZNodePinButtonWidget();

    QPushButton *button();

protected:
    QPushButton *m_btn;
};

//JZNodePinValueWidget
class JZCORE_EXPORT JZNodePinValueWidget : public JZNodePinWidget
{
    Q_OBJECT

public:
    JZNodePinValueWidget(QWidget *parent = nullptr);
    ~JZNodePinValueWidget();

    void initWidget(int data_type, QString widget_type = QString());
    QWidget *focusWidget();

    virtual QString value() const override;
    virtual void setValue(const QString &value) override;

protected slots:
    void onValueChanged();

protected:
    QString getWidgetType(int data_type);
    void createWidget();        
    void clearWidget();    

    QWidget *m_widget;

    int m_dataType;
    QString m_widgetType;
};

class JZCORE_EXPORT JZNodePinDisplayWidget : public JZNodePinWidget
{
    Q_OBJECT

public:
    JZNodePinDisplayWidget();

    void setRuntimeValue(const JZNodeDebugParamValue &value);
    virtual void updateWidget() override;

protected:
    virtual void init();

    void createWidget();
    QWidget *m_widget;
    int m_dataType;
};

#endif // !JZNODE_PARAM_WIDGET_H_
