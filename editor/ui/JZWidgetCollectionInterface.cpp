#include "qplugin.h"
#include "JZWidgetCollectionInterface.h"
#include "QImageLabel.h"
#include "3rd/qcustomplot/JZPlotConfg.h"

template<class T>
QWidget *createWidget(QWidget *parent)
{
    T *w = new T();
    w->setParent(parent);
    return w;
}

typedef QWidget *(*CreateWidgetFunc)(QWidget *parent);

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
    m_widgets.push_back(new JZWidgetInterface("QImageLabel", "Display Widgets", createWidget<QImageLabel>));
    m_widgets.push_back(new JZWidgetInterface("JZPlotWidget", "Display Widgets", createWidget<JZPlotWidget>));
}

JZWidgetCollectionInterface::~JZWidgetCollectionInterface()
{
    qDeleteAll(m_widgets);
}
 
QList<QDesignerCustomWidgetInterface*> JZWidgetCollectionInterface::customWidgets() const
{
    return m_widgets;
}