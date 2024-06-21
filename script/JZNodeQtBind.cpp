#include <QMetaProperty>
#include <QLineEdit>
#include <QDebug>
#include <QSlider>
#include <QComboBox>
#include "JZNodeQtBind.h"

BindObject::BindObject(QString widget,int widget_prop,QList<int> data_type,int dir)
{
    m_widget = widget;
    m_widgetProp = widget_prop;
    m_dataType = data_type;
    m_dir = dir;
}

BindObject::~BindObject()
{

}

QString BindObject::widget() const
{
    return m_widget;
}

int BindObject::widgetProp() const
{
    return m_widgetProp;
}

QList<int> BindObject::dataType() const
{
    return m_dataType;
}

int BindObject::dir() const
{
    return m_dir;
}

void BindObject::bind(QWidget *widget,QObject *object,QString path)
{
    //m_widget = widget;
    //m_data = object;
    //m_path = path;
    connect(widget,&QObject::destroyed,this,&BindObject::onWidgetDestroyed);
    connect(object,&QObject::destroyed,this,&BindObject::onObjectDestroyed);

    connectPropChanged(object,path);
    dataToUi(object,path,widget);
}

void BindObject::connectPropChanged(QObject *object,QString prop)
{   
    if(object->inherits("JZObject"))
    {
        connect(object,SIGNAL(valueChanged()),this,SLOT(onDataChanged()));
    }
    else
    {
        int prop_index = object->metaObject()->indexOfProperty(qUtf8Printable(prop));
        if(prop_index >= 0)
        {
            QMetaProperty meta = object->metaObject()->property(prop_index);
            auto method = "2" + meta.notifySignal().methodSignature();
            connect(object,method,this,SLOT(onDataChanged()));

            qDebug() << "connect" << method;
        }
        else
        {
            qDebug() << "no prop" << prop;
        }
    }
}
/*
int BindObject::indexOfWidget(QWidget *w)
{
    for(int i = m_bindList.size() - 1; i >= 0; i--)
    {
        if(m_bindList[i].widget == w)
            return i;
    }
    return -1;
}

int BindObject::indexOfData(QObject *obj)
{
    for(int i = m_bindList.size() - 1; i >= 0; i--)
    {
        if(m_bindList[i].content == obj)
            return i;
    }
    return -1;
}

void BindObject::onWidgetChanged()
{
    if(m_isChanged)
        return;
    qDebug() << "onWidgetChanged";

    auto w = qobject_cast<QWidget*>(sender());
    int index = indexOfWidget(w);
    auto &info = m_bindList[index];

    m_isChanged = true;
    uiToData(info.widget,info.content,info.prop);
    m_isChanged = false;
}

void BindObject::onDataChanged()
{   
    if(m_isChanged)
        return;

    m_isChanged = true;
    dataToUi(m_data,m_widget);
    m_isChanged = false;
}

void BindObject::onWidgetDestroyed(QObject *obj)
{   
    for(int i = m_bindList.size() - 1; i >= 0; i--)
    {
        if(m_bindList[i].widget == obj)
            m_bindList.removeAt(i);
    }
}

void BindObject::onObjectDestroyed(QObject *obj)
{
    for(int i = m_bindList.size() - 1; i >= 0; i--)
    {
        if(m_bindList[i].content == obj)
            m_bindList.removeAt(i);
    }
}

QList<QString> BindObject::extParams()
{
    return QList<QString>();
}

void BindObject::setExtParam(QList<QString> params)
{

}
*/

//LineEditBind
LineEditBind::LineEditBind()
    :BindObject("QLineEdit",Value,{0},Dir_duplex)
{

}
 
LineEditBind::~LineEditBind()
{

}

void LineEditBind::bind(QWidget *widget,QObject *object,QString path)
{
    BindObject::bind(widget,object,path);

    auto edit = qobject_cast<QLineEdit*>(widget);
    connect(edit,&QLineEdit::editingFinished,this,onWidgetChanged);
}

void LineEditBind::uiToData(QWidget *widget,QObject *content,QString path)
{
    auto edit = qobject_cast<QLineEdit*>(widget);
    QString name = edit->text();
    content->setProperty(qUtf8Printable(path),name);

    qDebug() << "edit->uiToData" << name;
}

void LineEditBind::dataToUi(QObject *content,QString path,QWidget *widget)
{
    auto edit = qobject_cast<QLineEdit*>(widget);
    QString name = content->property(qUtf8Printable(path)).toString();
    edit->setText(name);
    
    qDebug() << "edit->dataToUi" << name;
}

//SliderBind
SliderBind::SliderBind()
    :BindObject("QSlider",Value,{0},Dir_duplex)
{

}
 
SliderBind::~SliderBind()
{

}

void SliderBind::bind(QWidget *widget,QObject *object,QString path)
{
    BindObject::bind(widget,object,path);

    auto edit = qobject_cast<QSlider*>(widget);
    connect(edit,&QSlider::valueChanged,this,onWidgetChanged);
}

void SliderBind::uiToData(QWidget *widget,QObject *content,QString path)
{
    auto slider = qobject_cast<QSlider*>(widget);
    int value = slider->value();
    content->setProperty(qUtf8Printable(path),value);

    qDebug() << "SliderBind->uiToData" << value;
}

void SliderBind::dataToUi(QObject *content,QString path,QWidget *widget)
{
    auto slider = qobject_cast<QSlider*>(widget);
    int value = content->property(qUtf8Printable(path)).toInt();
    slider->setValue(value);
    
    qDebug() << "SliderBind->dataToUi" << value;
}

//ComboBoxBind
ComboBoxBind::ComboBoxBind()
    :BindObject("QComboBox",Value,{0},Dir_duplex)
{

}
 
ComboBoxBind::~ComboBoxBind()
{

}

void ComboBoxBind::bind(QWidget *widget,QObject *object,QString path)
{
    BindObject::bind(widget,object,path);

    auto edit = qobject_cast<QComboBox*>(widget);
    connect(edit,QOverload<int>::of(&QComboBox::currentIndexChanged),this,onWidgetChanged);
}

void ComboBoxBind::uiToData(QWidget *widget,QObject *content,QString path)
{
    auto box = qobject_cast<QComboBox*>(widget);
    int value = box->currentIndex();
    content->setProperty(qUtf8Printable(path),value);

    qDebug() << "BoxBind->uiToData" << value;
}

void ComboBoxBind::dataToUi(QObject *content,QString path,QWidget *widget)
{
    auto box = qobject_cast<QComboBox*>(widget);
    int value = content->property(qUtf8Printable(path)).toInt();
    box->setCurrentIndex(value);
    
    qDebug() << "BoxBind->dataToUi" << value;
}


//BindManager
BindManager *BindManager::instance()
{
    static BindManager inst;
    return &inst;
}

BindManager::BindManager()
{
}

BindManager::~BindManager()
{

}

void BindManager::regist(BindObject *bind)
{   
    bind->setParent(this);
    m_binds << bind;
}

BindObject *BindManager::bind(QWidget *w,int prop_type,QObject *context,QString prop,int dir)
{
    int data_type = 0;

    BindObject *bindObj = nullptr;
    for(int i = 0; i < m_binds.size(); i++)
    {
        auto bind = m_binds[i];
        if(w->inherits(qUtf8Printable(bind->widget()))
            && bind->widgetProp() == prop_type
            && bind->dataType().contains(data_type)
            && (bind->dir() & dir))
        {
            bindObj = bind;
            break;
        }
    }

    if(!bindObj)
    {
        qDebug() << "no bind for" << w->metaObject()->className();
        return nullptr;
    }

    bindObj->bind(w,context,prop);
    return bindObj;
}