#ifndef JZ_MENU_EXTENSION_INTERFACE_H_
#define JZ_MENU_EXTENSION_INTERFACE_H_

#include <QtDesigner>
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

class JZMenuExtension : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)

public:
    JZMenuExtension();
    virtual ~JZMenuExtension();
    virtual QList<QAction*> taskActions() const;
};

class JZMenuExtensionFactory :public QAbstractExtensionFactory
{
public:
    JZMenuExtensionFactory();
    virtual ~JZMenuExtensionFactory();
    virtual QObject *extension(QObject *object, const QString &iid) const;

    JZMenuExtension m_extMenu;
};

#endif