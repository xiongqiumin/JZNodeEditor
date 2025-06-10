#include <QBuffer>
#include <QWidget>
#include <QUiLoader>
#include "JZNodeUiLoader.h"
#include "JZNodeBind.h"
#include "modules/model/Yolo/JZYoloView.h"
#include "jzWidgets/JZImageLabel.h"
#include "jzWidgets/JZLogWidget.h"

//JZNodeWidgetManger
JZNodeWidgetManger *JZNodeWidgetManger::instance()
{
    static JZNodeWidgetManger inst;
    return &inst;
}

JZNodeWidgetManger::JZNodeWidgetManger()
{
    registWidget("JZImageLabel", createWidget<JZImageLabel>);
    registWidget("JZYoloView", createWidget<JZYoloView>);
    registWidget("JZLogWidget", createWidget<JZLogWidget>);
}

void JZNodeWidgetManger::registWidget(QString name, CreateWidgetFunc func)
{
    m_widgetMap[name] = func;
}

const QMap<QString, CreateWidgetFunc> &JZNodeWidgetManger::widgetMap()
{
    return m_widgetMap;
}

//JZNodeUiLoader
JZNodeObjectWidgetFactory JZNodeUiLoader::widgetFactory()
{
    JZNodeObjectWidgetFactory factory;
    factory.creator = [](JZNodeObject *obj) 
    {
        JZNodeUiLoader ui;
        ui.init(obj);
    };
    return factory;
}

JZNodeUiLoader::JZNodeUiLoader()
{

}

JZNodeUiLoader::~JZNodeUiLoader()
{
}

void JZNodeUiLoader::init(JZNodeObject *jz_obj)
{
    QWidget *obj = (QWidget*)(jz_obj->cobj());
    auto inst = jz_obj->manager();
    auto def = jz_obj->meta();

    QString xml = QString::fromUtf8(def->widgetDefine.buffer);
    create(obj, xml);
    
    for (int i = 0; i < def->widgetParams.size(); i++)
    {
        auto &param_def = def->widgetParams[i];

        QWidget *w = obj->findChild<QWidget*>(param_def.name);
        if (w)
        {
            JZNodeObject *jzobj = inst->createReference(param_def.type,w, false);            
            JZNodeObjectPointer ptr(jzobj, true);
            QVariantPtr qptr;
            qptr.type = jzobj->type();
            *qptr.ptr = QVariant::fromValue(ptr);
            
            jz_obj->initParam(param_def.name, qptr);
        }
    }
}

void JZNodeUiLoader::create(QWidget *w,QString xml)
{
    m_widget = w;

    QBuffer buffer;
    QByteArray data = xml.toUtf8();
    buffer.setData(data);
    QUiLoader::load(&buffer);
}

QWidget *JZNodeUiLoader::createWidget(const QString &className, QWidget *parent, const QString &name)
{
    if (parent == nullptr)
        return m_widget; 

    auto &widget_map = JZNodeWidgetManger::instance()->widgetMap();
    if (widget_map.contains(className))
    {
        auto w = widget_map[className](parent);
        w->setObjectName(name);
        return w;
    }

    return QUiLoader::createWidget(className, parent, name);
}
