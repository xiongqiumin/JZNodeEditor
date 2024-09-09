#ifndef JZNODE_VARIABLE_BIND_H_
#define JZNODE_VARIABLE_BIND_H_

#include <QWidget>
#include <QMap>
#include <QVariant>
#include <functional>
#include "JZNodeType.h"

enum WidgetProp{        
    WidgetProp_Value,
    WidgetProp_Index,
};

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
    QList<int> dataType;
};

class BindObject : public QObject
{
    Q_OBJECT

public:
    BindObject(QString widget,int widget_prop,QList<int> data_type,int dir);
    virtual ~BindObject();    
    
    virtual void bind(QWidget *widget,QObject *object,QString path);
    void uiToData();
    void dataToUi();    

protected slots:    
    void onWidgetChanged();
    void onDataChanged();

protected:
    void connectPropChanged(QObject *object,QString prop);
    int variableType(const QString &path);
    QVariant getVariable(const QString &path);
    void setVariable(const QString &path, const QVariant &value);

    virtual void uiToDataImpl() = 0;
    virtual void dataToUiImpl() = 0;

    QWidget *m_widget;
    QObject *m_context;
    QString m_path;
    int m_dataType;
    bool m_changed;
};

//LineEditBind
class LineEditBind : public BindObject
{
    Q_OBJECT

public:
    LineEditBind();
    virtual ~LineEditBind();

protected slots:
    virtual void bind(QWidget *widget,QObject *object,QString prop) override;
    virtual void uiToDataImpl() override;
    virtual void dataToUiImpl() override;
};

//SliderBind
class SliderBind : public BindObject
{
    Q_OBJECT

public:
    SliderBind();
    virtual ~SliderBind();

protected slots:
    virtual void bind(QWidget *widget,QObject *object,QString prop) override;
    virtual void uiToDataImpl() override;
    virtual void dataToUiImpl() override;
};

//ComboBoxBind
class ComboBoxBind : public BindObject
{
    Q_OBJECT

public:
    ComboBoxBind();
    virtual ~ComboBoxBind();

protected slots:
    virtual void bind(QWidget *widget,QObject *object,QString prop) override;
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
    virtual BindObject *createBind(QString class_name) = 0;


protected:
    QMap<QString, BindInfo> m_widgetMap;
};

//QtBindFactory
class QtBindFactory : public BindFactory
{
public:
    QtBindFactory();

    virtual BindObject *createBind(QString class_name);
};

//BindManager
class BindManager : public QObject
{
    Q_OBJECT

public:
    static BindManager *instance();

    BindManager();
    ~BindManager();

    void init();
    void regist(BindFactory *factory);

    BindObject *bind(QWidget *w,int prop_type,QObject *context,QString prop,int dir);

protected:
    QList<BindFactory*> m_binds;
};

#endif // !JZNODE_QT_BIND_H_
