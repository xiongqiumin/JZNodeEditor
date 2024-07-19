#ifndef JZNODE_PARAM_WIDGET_H_
#define JZNODE_PARAM_WIDGET_H_

#include <QWidget>
#include <QComboBox>
#include "JZProcess.h"
#include "JZNodeDebugPacket.h"

class JZNodeFlagEditWidget : public QWidget
{
    Q_OBJECT

public:    
    JZNodeFlagEditWidget(QWidget *parent = nullptr);
    ~JZNodeFlagEditWidget();

    void setFlagType(QString flag);
    QString flagType();

    QString value();
    void setValue(QString text);    

signals:
    void sigValueChanged();

protected slots:
    void onSettingClicked();

protected:
    QLineEdit *m_line;
    QString m_flagType;    
};

class JZNodeParamTypeWidget : public QComboBox
{
    Q_OBJECT

public:
    JZNodeParamTypeWidget(QWidget *parent = nullptr);

    QString type();
    void setType(QString type);

signals:
    void sigTypeChanged();

protected:
};

class JZNode;
class JZNodePinWidget : public QWidget
{
    Q_OBJECT
        
public:
    JZNodePinWidget(QWidget *parent = nullptr);
    ~JZNodePinWidget();
    
    virtual QString value() const;
    virtual void setValue(const QString &value);

signals:
    void sigValueChanged(const QString &value);
};

//JZNodePinButtonWidget
class QPushButton;
class JZNodePinButtonWidget : public JZNodePinWidget
{
    Q_OBJECT

public:
    JZNodePinButtonWidget();
    ~JZNodePinButtonWidget();

    QPushButton *button();

protected:
    QPushButton *m_btn;
};

//JZNodeParamValueWidget
class JZNodeParamValueWidget : public JZNodePinWidget
{
    Q_OBJECT

public:
    JZNodeParamValueWidget(QWidget *parent = nullptr);
    ~JZNodeParamValueWidget();

    void initWidget(int data_type, QString widget_type = QString());
    QWidget *widget();

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



#endif // !JZNODE_PARAM_WIDGET_H_
