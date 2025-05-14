#include <QBuffer>
#include <QWidget>
#include <QUiLoader>
#include "JZNodeUiLoader.h"
#include "JZNodeBind.h"
#include "modules/model/JZYoloView.h"
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
JZNodeUiLoader::JZNodeUiLoader()
{

}

JZNodeUiLoader::~JZNodeUiLoader()
{
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
