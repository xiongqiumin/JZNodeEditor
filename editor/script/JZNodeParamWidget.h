#ifndef JZNODE_PARAM_WIDGET_H_
#define JZNODE_PARAM_WIDGET_H_

#include <QWidget>
#include <QComboBox>
#include "JZProcess.h"
#include "JZNodeDebugPacket.h"
#include "QImageLabel.h"

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

//JZNodeParamEditWidget
class JZNodeParamEditWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamEditWidget();
    virtual ~JZNodeParamEditWidget();

    virtual void init();
    virtual QString value() = 0;
    virtual void setValue(const QString &text) = 0;

signals:
    void sigValueChanged();
};
typedef JZNodeParamEditWidget *(*CreateParamEditFunc)();
typedef QVariant(*CreateParamFunc)(const QString &value);
template<class T>
JZNodeParamEditWidget *CreateParamEditWidget() { return new T(); }

//JZNodeParamDiaplayWidget
class JZNodeParamDiaplayWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamDiaplayWidget();
    virtual ~JZNodeParamDiaplayWidget();

protected:
    void init();
};
typedef JZNodeParamDiaplayWidget *(*CreateParamDiaplayFunc)();
template<class T>
JZNodeParamDiaplayWidget *CreateParamDiaplayWidget() { return new T(); }

//JZNodePinWidget
class JZNode;
class JZNodePinWidget : public QWidget
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

class JZNodeDisplayWidget : public JZNodePinWidget
{
    Q_OBJECT

public:
    JZNodeDisplayWidget();

    void setRuntimeValue(const JZNodeDebugParamValue &value);
    virtual void updateWidget() override;

protected:
    virtual void init();

    void createWidget();
    QWidget *m_widget;
};

class JZNodeParamWidgetManager
{
public:
    static JZNodeParamWidgetManager *instance();

    void registEditWidget(int edit_type, CreateParamEditFunc func);
    void registEditDelegate(int data_type, int edit_type, CreateParamFunc func);
    void registDisplayWidget(int data_type, CreateParamDiaplayFunc func);

    bool hasEditDelegate(int data_type);
    bool hasEditWidget(int edit_type);
    bool hasDisplayWidget(int data_type);
    
    int editDelegate(int data_type);
    JZNodeParamEditWidget *createEditWidget(int edit_type);
    QVariant createEditParam(int data_type ,const QString &value);

    JZNodeParamDiaplayWidget *createDisplayWidget(int data_type);

protected:
    struct EditDelegate
    {
        int editType;
        CreateParamFunc func;
    };

    JZNodeParamWidgetManager();
    ~JZNodeParamWidgetManager();

    QMap<int, EditDelegate> m_editDelegate;
    QMap<int, CreateParamEditFunc> m_editFactory;
    QMap<int, CreateParamDiaplayFunc> m_displayFactory;
};

#endif // !JZNODE_PARAM_WIDGET_H_
