#ifndef JZNODE_UI_LOADER_H_
#define JZNODE_UI_LOADER_H_

#include <QUiLoader>
#include <QMap>

template<class T>
QWidget *createWidget(QWidget *parent)
{
    T *w = new T();
    w->setParent(parent);
    return w;
}
typedef QWidget *(*CreateWidgetFunc)(QWidget *parent);

class JZNodeWidgetManger
{
public:
    static JZNodeWidgetManger *instance();

    void registWidget(QString name, CreateWidgetFunc func);
    const QMap<QString, CreateWidgetFunc> &widgetMap();

protected:
    JZNodeWidgetManger();

    QMap<QString, CreateWidgetFunc> m_widgetMap;
};

class JZNodeUiLoader : public QUiLoader
{
    Q_OBJECT

public:
    JZNodeUiLoader();
    ~JZNodeUiLoader();

    QWidget *create(QString text);

private:
    virtual QWidget *createWidget(const QString &className, QWidget *parent = Q_NULLPTR, const QString &name = QString()) override;
};

#endif
