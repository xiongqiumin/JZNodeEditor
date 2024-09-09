#include <QMetaProperty>
#include <QLineEdit>
#include <QDebug>
#include <QSlider>
#include <QComboBox>
#include "JZNodeVariableBind.h"
#include "JZNodeObject.h"
#include "JZRegExpHelp.h"

BindObject::BindObject(QString widget,int widget_prop,QList<int> data_type,int dir)
{
    m_dataType = Type_none;
    m_widget = nullptr;
    m_context = nullptr;
    m_changed = false;
}

BindObject::~BindObject()
{

}

void BindObject::bind(QWidget *widget,QObject *object,QString path)
{
    m_widget = widget;
    m_context = object;
    m_path = path;
    m_dataType = variableType(path);

    connectPropChanged(object,path);
    dataToUi();
}

void BindObject::connectPropChanged(QObject *object, QString prop)
{
    if (object->inherits("JZNodeObject"))
    {
        auto jz_obj = qobject_cast<JZNodeObject*>(object);
        connect(jz_obj, &JZNodeObject::sigValueChanged, this, [this,prop](const QString &name) {
            if(name == prop)
                this->onDataChanged();
        });
    }
    else
    {        
        int prop_index = object->metaObject()->indexOfProperty(qUtf8Printable(prop));
        if (prop_index >= 0)
        {
            QMetaProperty meta = object->metaObject()->property(prop_index);
            auto method = "2" + meta.notifySignal().methodSignature();
            connect(object, method, this, SLOT(onDataChanged()));

            qDebug() << "connect" << method;
        }
        else
        {
            qDebug() << "no prop" << prop;
        }
    }    
}

int BindObject::variableType(const QString &path)
{
    if (m_context->inherits("JZNodeObject"))
    {
        auto jz_obj = qobject_cast<JZNodeObject*>(m_context);
        return jz_obj->meta()->param(path)->dataType();
    }
    else
    {
        int idx = m_context->metaObject()->indexOfProperty(qUtf8Printable(path));
        auto prop_meta = m_context->metaObject()->property(idx);
        Q_ASSERT(0);
        return Type_none;
    }
}

QVariant BindObject::getVariable(const QString &path)
{
    if (m_context->inherits("JZNodeObject"))
    {
        auto jz_obj = qobject_cast<JZNodeObject*>(m_context);
        return jz_obj->param(path);
    }
    else
    {
        return m_context->property(qUtf8Printable(path));
    }
}

void BindObject::setVariable(const QString &path, const QVariant &value)
{
    QVariant v = JZNodeType::convertTo(m_dataType, value);
    if (m_context->inherits("JZNodeObject"))
    {
        auto jz_obj = qobject_cast<JZNodeObject*>(m_context);
        jz_obj->setParam(path,v);
    }
    else
    {
        m_context->setProperty(qUtf8Printable(path),v);
    }
}

void BindObject::uiToData()
{
    if (m_changed)
        return;

    m_changed = true;
    uiToDataImpl();
    m_changed = false;
}


void BindObject::dataToUi()
{
    if (m_changed)
        return;

    m_changed = true;
    dataToUiImpl();
    m_changed = false;
}

void BindObject::onWidgetChanged()
{
    uiToData();    
}

void BindObject::onDataChanged()
{   
    dataToUi();
}

//LineEditBind
LineEditBind::LineEditBind()
    :BindObject("QLineEdit",WidgetProp_Value,{0},Dir_duplex)
{

}
 
LineEditBind::~LineEditBind()
{

}

void LineEditBind::bind(QWidget *widget,QObject *object,QString path)
{
    BindObject::bind(widget,object,path);

    auto edit = qobject_cast<QLineEdit*>(widget);
    connect(edit,&QLineEdit::editingFinished,this,&LineEditBind::onWidgetChanged);
}

void LineEditBind::uiToDataImpl()
{
    auto edit = qobject_cast<QLineEdit*>(m_widget);
    QString value = edit->text();
    if (JZNodeType::isNumber(m_dataType) && !JZRegExpHelp::isNumber(value))
    {
        edit->setText("0");
        value = "0";
    }
    setVariable(m_path, value);
}

void LineEditBind::dataToUiImpl()
{
    auto edit = qobject_cast<QLineEdit*>(m_widget);
    QString name = getVariable(m_path).toString();
    edit->setText(name);    
}

//SliderBind
SliderBind::SliderBind()
    :BindObject("QSlider", WidgetProp_Value,{0},Dir_duplex)
{

}
 
SliderBind::~SliderBind()
{

}

void SliderBind::bind(QWidget *widget,QObject *object,QString path)
{
    BindObject::bind(widget,object,path);

    auto edit = qobject_cast<QSlider*>(widget);
    connect(edit,&QSlider::valueChanged,this,&SliderBind::onWidgetChanged);
}

void SliderBind::uiToDataImpl()
{
    auto slider = qobject_cast<QSlider*>(m_widget);
    int value = slider->value();
    setVariable(m_path,value);
}

void SliderBind::dataToUiImpl()
{
    auto slider = qobject_cast<QSlider*>(m_widget);
    int value = getVariable(m_path).toInt();
    slider->setValue(value);   
}

//ComboBoxBind
ComboBoxBind::ComboBoxBind()
    :BindObject("QComboBox", WidgetProp_Value,{0},Dir_duplex)
{

}
 
ComboBoxBind::~ComboBoxBind()
{

}

void ComboBoxBind::bind(QWidget *widget,QObject *object,QString path)
{
    BindObject::bind(widget,object,path);

    auto edit = qobject_cast<QComboBox*>(widget);
    connect(edit,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&ComboBoxBind::onWidgetChanged);
}

void ComboBoxBind::uiToDataImpl()
{
    Q_ASSERT(0);
}

void ComboBoxBind::dataToUiImpl()
{
    Q_ASSERT(0);
}

//BindFactory
BindFactory::BindFactory()
{
}

BindFactory::~BindFactory()
{
}

const QMap<QString, BindInfo> &BindFactory::widgetMap() const
{
    return m_widgetMap;
}

//QtBindFactory
QtBindFactory::QtBindFactory()
{
    m_widgetMap["QLineEdit"].dataType << Type_int << Type_double << Type_string;
    m_widgetMap["QComboxBox"].dataType << Type_int << Type_string;
    m_widgetMap["QSlider"].dataType << Type_int;
}

BindObject *QtBindFactory::createBind(QString class_name)
{
    if (class_name == "QLineEdit")
        return new LineEditBind();
    else if (class_name == "QComboxBox")
        new ComboBoxBind();
    else if (class_name == "QSlider")
        new SliderBind();

    return nullptr;
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

void BindManager::init()
{
    regist(new QtBindFactory());
}

void BindManager::regist(BindFactory *factory)
{   
    factory->setParent(this);
    m_binds << factory;
}

BindObject *BindManager::bind(QWidget *w,int widget_prop,QObject *context,QString prop,int dir)
{    
    BindObject *bindObj = nullptr;
    for(int i = 0; i < m_binds.size(); i++)
    {
        auto bind = m_binds[i];
        auto &map = bind->widgetMap();
        auto it = map.begin();
        while (it != map.end())
        {
            if (w->inherits(qUtf8Printable(it.key())))
            {
                bindObj = bind->createBind(it.key());
                goto create_end;
            }
            it++;
        }        
    }

create_end:
    if(!bindObj)
    {
        qDebug() << "no bind for" << w->metaObject()->className();
        return nullptr;
    }

    bindObj->bind(w,context,prop);
    return bindObj;
}