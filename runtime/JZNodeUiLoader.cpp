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

QWidget *JZNodeUiLoader::create(QString xml)
{    
    QBuffer buffer;
    QByteArray data = xml.toUtf8();
    buffer.setData(data);
    return QUiLoader::load(&buffer);
}

QWidget *JZNodeUiLoader::createWidget(const QString &className, QWidget *parent, const QString &name)
{
    if (className == "QWidget" && parent == nullptr)
    {
        auto *w = new jzbind::WidgetWrapper<QWidget>();
        w->setObjectName(name);
        return w;
    }    

    auto &widget_map = JZNodeWidgetManger::instance()->widgetMap();
    if (widget_map.contains(className))
    {
        auto w = widget_map[className](parent);
        w->setObjectName(name);
        return w;
    }

    return QUiLoader::createWidget(className, parent, name);
}
