#include <QMetaProperty>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QSlider>
#include <QComboBox>
#include <QMetaType>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "JZNodeVariableBind.h"
#include "JZNodeObject.h"
#include "JZRegExpHelp.h"
#include "JZNodeEngine.h"

JZBindObject::JZBindObject()
{
    m_dataType = Type_none;
    m_widget = nullptr;
    m_context = nullptr;
    m_changed = false;
}

JZBindObject::~JZBindObject()
{

}

const JZScriptEnvironment* JZBindObject::environment()
{
    return m_context->manager()->env();
}

void JZBindObject::bind(QWidget *widget, JZNodeObject *object,QString param)
{
    m_widget = widget;
    m_context = object;
    m_param = param;
    m_dataType = variableType(param);

    dataToUi();
}

int JZBindObject::variableType(const QString & param)
{
    auto env = m_context->meta()->manager->env();
    return env->nameToType(m_context->meta()->param(param)->type);
}

QVariant JZBindObject::getVariable(const QString & param)
{
    return m_context->param(param);
}

void JZBindObject::setVariable(const QString & param, const QVariant &value)
{
    m_context->setParam(param,value);
}

void JZBindObject::uiToData()
{
    if (m_changed)
        return;

    m_changed = true;
    uiToDataImpl();
    m_changed = false;
}


void JZBindObject::dataToUi()
{
    if (m_changed)
        return;

    m_changed = true;
    dataToUiImpl();
    m_changed = false;
}

void JZBindObject::onWidgetChanged()
{
    uiToData();    
}

//LabelBind
LabelBind::LabelBind()
{
}

LabelBind::~LabelBind()
{

}

void LabelBind::bind(QWidget* widget, JZNodeObject* object, QString path)
{
    JZBindObject::bind(widget, object, path);
}

void LabelBind::uiToDataImpl()
{
    auto edit = qobject_cast<QLabel*>(m_widget);
    QString text = edit->text();
    QVariant value = environment()->tryConvertTo(text, m_dataType);
    if (!value.isValid())
        value = environment()->defaultValue(m_dataType);
    setVariable(m_param, value);
}

void LabelBind::dataToUiImpl()
{
    auto edit = qobject_cast<QLabel*>(m_widget);
    QString name = getVariable(m_param).toString();
    edit->setText(name);
}

//LineEditBind
LineEditBind::LineEditBind()
{
}
 
LineEditBind::~LineEditBind()
{

}

void LineEditBind::bind(QWidget *widget, JZNodeObject *object,QString path)
{
    JZBindObject::bind(widget,object,path);

    auto edit = qobject_cast<QLineEdit*>(widget);
    connect(edit,&QLineEdit::textChanged, this, &LineEditBind::onWidgetChanged);
}

void LineEditBind::uiToDataImpl()
{
    auto edit = qobject_cast<QLineEdit*>(m_widget);
    QString text = edit->text();
    QVariant value = environment()->tryConvertTo(text, m_dataType);
    if (!value.isValid())
        value = environment()->defaultValue(m_dataType);
    setVariable(m_param, value);
}

void LineEditBind::dataToUiImpl()
{
    auto edit = qobject_cast<QLineEdit*>(m_widget);
    QString name = getVariable(m_param).toString();
    edit->setText(name);    
}

//SliderBind
SliderBind::SliderBind()
{

}
 
SliderBind::~SliderBind()
{

}

void SliderBind::bind(QWidget *widget, JZNodeObject *object,QString path)
{
    JZBindObject::bind(widget,object,path);

    auto edit = qobject_cast<QSlider*>(widget);
    connect(edit,&QSlider::valueChanged,this,&SliderBind::onWidgetChanged);
}

void SliderBind::uiToDataImpl()
{
    auto slider = qobject_cast<QSlider*>(m_widget);
    int value = slider->value();
    setVariable(m_param,value);
}

void SliderBind::dataToUiImpl()
{
    auto slider = qobject_cast<QSlider*>(m_widget);
    int value = getVariable(m_param).toInt();
    slider->setValue(value);   
}

//SpinBoxBind
SpinBoxBind::SpinBoxBind()
{

}

SpinBoxBind::~SpinBoxBind()
{

}

void SpinBoxBind::bind(QWidget* widget, JZNodeObject* object, QString path)
{
    JZBindObject::bind(widget, object, path);

    auto edit = qobject_cast<QSpinBox*>(widget);
    connect(edit, QOverload<int>::of(&QSpinBox::valueChanged), this, &SpinBoxBind::onWidgetChanged);
}

void SpinBoxBind::uiToDataImpl()
{
    auto slider = qobject_cast<QSpinBox*>(m_widget);
    int value = slider->value();
    setVariable(m_param, value);
}

void SpinBoxBind::dataToUiImpl()
{
    auto slider = qobject_cast<QSpinBox*>(m_widget);
    int value = getVariable(m_param).toInt();
    slider->setValue(value);
}

//DoubleSpinBoxBind
DoubleSpinBoxBind::DoubleSpinBoxBind()
{

}

DoubleSpinBoxBind::~DoubleSpinBoxBind()
{

}

void DoubleSpinBoxBind::bind(QWidget* widget, JZNodeObject* object, QString path)
{
    JZBindObject::bind(widget, object, path);

    auto edit = qobject_cast<QDoubleSpinBox*>(widget);
    connect(edit, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DoubleSpinBoxBind::onWidgetChanged);
}

void DoubleSpinBoxBind::uiToDataImpl()
{
    auto slider = qobject_cast<QDoubleSpinBox*>(m_widget);
    double value = slider->value();
    setVariable(m_param, value);
}

void DoubleSpinBoxBind::dataToUiImpl()
{
    auto slider = qobject_cast<QDoubleSpinBox*>(m_widget);
    double value = getVariable(m_param).toDouble();
    slider->setValue(value);
}

//ComboBoxBind
ComboBoxBind::ComboBoxBind()
{

}
 
ComboBoxBind::~ComboBoxBind()
{

}

void ComboBoxBind::bind(QWidget *widget, JZNodeObject *object,QString path)
{
    JZBindObject::bind(widget,object,path);

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
    m_widgetMap["QLabel"].dataType << Type_int << Type_double << Type_string;
    m_widgetMap["QLineEdit"].dataType << Type_int << Type_double << Type_string;
    m_widgetMap["QComboxBox"].dataType << Type_int << Type_string;
    m_widgetMap["QSlider"].dataType << Type_int;
    m_widgetMap["QSpinBox"].dataType << Type_int;
    m_widgetMap["QDoubleSpinBox"].dataType << Type_double;
}

JZBindObject *QtBindFactory::createBind(QString class_name)
{
    if (class_name == "QLabel")
        return new LabelBind();
    else if (class_name == "QLineEdit")
        return new LineEditBind();
    else if (class_name == "QComboxBox")
        return new ComboBoxBind();
    else if (class_name == "QSlider")
        return new SliderBind();
    else if (class_name == "QSpinBox")
        return new SpinBoxBind();
    else if (class_name == "QDoubleSpinBox")
        return new DoubleSpinBoxBind();

    return nullptr;
}

//JZBindManager
JZBindManager *JZBindManager::instance()
{
    static JZBindManager inst;
    return &inst;
}

JZBindManager::JZBindManager()
{
}

JZBindManager::~JZBindManager()
{

}

void JZBindManager::init()
{
    regist(new QtBindFactory());
}

void JZBindManager::regist(BindFactory *factory)
{   
    factory->setParent(this);
    m_binds << factory;
}

QList<int> JZBindManager::supportBind(QString widget_class,int data_type)
{
    QSet<int> support_type;
    //
    int typeId = QMetaType::type(qUtf8Printable(widget_class));
    if (typeId == QMetaType::UnknownType)
        return support_type.values();

    const QMetaObject *meta_obj = QMetaType::metaObjectForType(typeId);
    for(int i = 0; i < m_binds.size(); i++)
    {
        auto bind = m_binds[i];
        auto &map = bind->widgetMap();
        auto it = map.begin();
        while (it != map.end())
        {
            int super_id = QMetaType::type(qUtf8Printable(it.key()));
            const QMetaObject* super_meta = QMetaType::metaObjectForType(typeId);
            if (meta_obj->inherits(super_meta))
            {
                for(auto d : it->dataType)
                    support_type.insert(d);
            }
            it++;
        }        
    }

    return support_type.values();
}

JZBindObject *JZBindManager::bind(QWidget *w,JZNodeObject *context,QString prop,int dir)
{    
    JZBindObject *bindObj = nullptr;
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
        Q_ASSERT(0);
        return nullptr;
    }

    bindObj->bind(w,context,prop);
    if (dir & Dir_dataToUi)
        context->addBind(prop, bindObj);

    return bindObj;
}