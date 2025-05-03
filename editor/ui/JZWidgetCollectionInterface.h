#ifndef JZ_WIDGET_COLLECTION_INTERFACE_H_
#define JZ_WIDGET_COLLECTION_INTERFACE_H_

#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

#define QT_STATICPLUGIN
class JZWidgetCollectionInterface : public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QDesignerCustomWidgetCollectionInterface_iid)
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    JZWidgetCollectionInterface(QObject* parent = 0);
    ~JZWidgetCollectionInterface();

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const override;

protected:
    QList<QDesignerCustomWidgetInterface*> m_widgets;
};

#endif