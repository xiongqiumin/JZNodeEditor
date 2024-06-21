#ifndef JZNODE_QT_BIND_H_
#define JZNODE_QT_BIND_H_

#include <QWidget>
#include <QMap>
#include <QVariant>
#include <functional>
#include "JZNodeType.h"

enum{
    Enabled,
    Visible,
    Pos,
    Size,
    Checked,
    Index,
    Value,
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

class BindObject : public QObject
{
    Q_OBJECT

public:
    BindObject(QString widget,int widget_prop,QList<int> data_type,int dir);
    virtual ~BindObject();

    QString widget() const;
    int widgetProp() const;
    QList<int> dataType() const;
    int dir() const;
    
    virtual void bind(QWidget *widget,QObject *object,QString path);
    virtual void uiToData(QWidget *w,QObject *content,QString path) = 0;
    virtual void dataToUi(QObject *content,QString path,QWidget *w) = 0;
    QList<QString> extParams(){ return QList<QString>();};
    void setExtParam(QList<QString> params){};

protected slots:
    void onWidgetDestroyed(QObject *obj){};
    void onObjectDestroyed(QObject *obj){};
    
    void onWidgetChanged(){};
    void onDataChanged(){};

protected:
    void connectPropChanged(QObject *object,QString prop);

    QString m_widget;
    int m_widgetProp;
    QList<int> m_dataType;
    int m_dir;
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
    virtual void uiToData(QWidget *w,QObject *content,QString prop) override;
    virtual void dataToUi(QObject *content,QString prop,QWidget *w) override;
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
    virtual void uiToData(QWidget *w,QObject *content,QString prop) override;
    virtual void dataToUi(QObject *content,QString prop,QWidget *w) override;
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
    virtual void uiToData(QWidget *w,QObject *content,QString prop) override;
    virtual void dataToUi(QObject *content,QString prop,QWidget *w) override;
};

//BindManager
class BindManager : public QObject
{
    Q_OBJECT

public:
    static BindManager *instance();

    BindManager();
    ~BindManager();

    void regist(BindObject *bind);

    BindObject *bind(QWidget *w,int prop_type,QObject *context,QString prop,int dir);

protected:
    QList<BindObject*> m_binds;
};

#endif // !JZNODE_QT_BIND_H_
