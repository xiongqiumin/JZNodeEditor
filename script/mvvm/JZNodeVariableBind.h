#ifndef JZNODE_VARIABLE_BIND_H_
#define JZNODE_VARIABLE_BIND_H_

#include <QWidget>
#include <QMap>
#include <QVariant>
#include <functional>
#include "JZNodeType.h"
#include "JZScriptEnvironment.h"

enum{
    Dir_none,
    Dir_dataToUi = 0x1,
    Dir_uiToData = 0x2,
    Dir_duplex = (Dir_dataToUi | Dir_uiToData),
};

enum{
    Trigger_valueChanged,
    Trigger_editFinish,
};

class BindInfo
{
public:
    QString widget;
    QList<int> dataType;   //支持的数据类型
};

class JZBindManager;
class JZBindObject : public QObject
{
    Q_OBJECT

public:
    JZBindObject();
    virtual ~JZBindObject();
    
    void uiToData();
    void dataToUi();    

protected slots:    
    void onWidgetChanged();

protected:
    friend JZBindManager;

    virtual void bind(QWidget* widget, JZNodeObject* object, QString param);
    int variableType(const QString & param);
    QVariant getVariable(const QString & param);
    void setVariable(const QString & param, const QVariant &value);
    const JZScriptEnvironment* environment();

    virtual void uiToDataImpl() = 0;
    virtual void dataToUiImpl() = 0;

    QWidget *m_widget;
    JZNodeObject *m_context;
    QString m_param;
    int m_dataType;
    bool m_changed;
};

//LabelBind
class LabelBind : public JZBindObject
{
    Q_OBJECT

public:
    LabelBind();
    virtual ~LabelBind();

protected slots:
    virtual void bind(QWidget* widget, JZNodeObject* object, QString prop) override;
    virtual void uiToDataImpl() override;
    virtual void dataToUiImpl() override;
};

//LineEditBind
class LineEditBind : public JZBindObject
{
    Q_OBJECT

public:
    LineEditBind();
    virtual ~LineEditBind();

protected slots:
    virtual void bind(QWidget *widget, JZNodeObject*object,QString prop) override;
    virtual void uiToDataImpl() override;
    virtual void dataToUiImpl() override;
};

//SliderBind
class SliderBind : public JZBindObject
{
    Q_OBJECT

public:
    SliderBind();
    virtual ~SliderBind();

protected slots:
    virtual void bind(QWidget *widget, JZNodeObject*object,QString prop) override;
    virtual void uiToDataImpl() override;
    virtual void dataToUiImpl() override;
};

//SpinBoxBind
class SpinBoxBind : public JZBindObject
{
    Q_OBJECT

public:
    SpinBoxBind();
    virtual ~SpinBoxBind();

protected slots:
    virtual void bind(QWidget* widget, JZNodeObject* object, QString prop) override;
    virtual void uiToDataImpl() override;
    virtual void dataToUiImpl() override;
};

//DoubleSpinBoxBind
class DoubleSpinBoxBind : public JZBindObject
{
    Q_OBJECT

public:
    DoubleSpinBoxBind();
    virtual ~DoubleSpinBoxBind();

protected slots:
    virtual void bind(QWidget* widget, JZNodeObject* object, QString prop) override;
    virtual void uiToDataImpl() override;
    virtual void dataToUiImpl() override;
};

//ComboBoxBind
class ComboBoxBind : public JZBindObject
{
    Q_OBJECT

public:
    ComboBoxBind();
    virtual ~ComboBoxBind();

protected slots:
    virtual void bind(QWidget *widget, JZNodeObject *object,QString prop) override;
    virtual void uiToDataImpl() override;
    virtual void dataToUiImpl() override;
};

//BindFactory
class BindFactory : public QObject
{    
public:
    BindFactory();
    virtual ~BindFactory();

    const QMap<QString, BindInfo> &widgetMap() const;
    virtual JZBindObject *createBind(QString class_name) = 0;


protected:
    QMap<QString, BindInfo> m_widgetMap;
};

//QtBindFactory
class QtBindFactory : public BindFactory
{
public:
    QtBindFactory();

    virtual JZBindObject *createBind(QString class_name);
};

//JZBindManager
class JZBindManager : public QObject
{
    Q_OBJECT

public:
    static JZBindManager*instance();

    JZBindManager();
    ~JZBindManager();

    void init();
    void regist(BindFactory *factory);

    QList<int> supportBind(QString widget_class, int data_type);
    JZBindObject *bind(QWidget *w,JZNodeObject *context,QString prop,int dir);

protected:
    QList<BindFactory*> m_binds;
};

#endif // !JZNODE_QT_BIND_H_
