#ifndef JZNODE_UI_LOADER_H_
#define JZNODE_UI_LOADER_H_

#include <QUiLoader>

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
