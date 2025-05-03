#include "JZWidgetCollectionInterface.h"
#include "qplugin.h"
#include "runtime/JZNodeUiLoader.h"

class JZWidgetInterface :public QDesignerCustomWidgetInterface
{    
public:
    JZWidgetInterface(QString name, QString group, CreateWidgetFunc func)
    {
        m_name = name;
        m_group = group;
        m_create = func;
    }
    virtual ~JZWidgetInterface()
    {
    }
    virtual QString name() const
    {
        return m_name;
    }
    virtual QString group() const
    {
        return m_group;
    }
    virtual QString toolTip() const
    {
        return QString();
    }
    virtual QString whatsThis() const 
    {
        return QString();
    }
    virtual QString includeFile() const
    {
        return QString();
    }
    virtual QIcon icon() const
    {
        return QIcon();
    }
    virtual bool isContainer() const
    {
        return false;
    }

    virtual QWidget *createWidget(QWidget *parent)
    {        
        return m_create(parent);
    }

    CreateWidgetFunc m_create;
    QString m_name;
    QString m_group;
};

Q_IMPORT_PLUGIN(JZWidgetCollectionInterface)
JZWidgetCollectionInterface::JZWidgetCollectionInterface(QObject* parent)
    :QObject(parent)
{    
    auto &widget_map = JZNodeWidgetManger::instance()->widgetMap();
    auto it = widget_map.begin();
    while(it != widget_map.end())
    {
        m_widgets.push_back(new JZWidgetInterface(it.key(), "Advance Widgets", it.value()));

        it++;
    }
}

JZWidgetCollectionInterface::~JZWidgetCollectionInterface()
{
    qDeleteAll(m_widgets);
}
 
QList<QDesignerCustomWidgetInterface*> JZWidgetCollectionInterface::customWidgets() const
{
    return m_widgets;
}

#include "moc_JZWidgetCollectionInterface.cpp"