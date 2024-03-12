#ifndef JZNODE_QT_BIND_H_
#define JZNODE_QT_BIND_H_

#include <QWidget>
#include "JZNodeType.h"

class JZNodeQtBindHelper : public QObject
{
    Q_OBJECT

public:
    JZNodeQtBindHelper(QWidget *parent);
    ~JZNodeQtBindHelper();

    void blockChanged(bool flag);

public slots:
    void valueChanged();

protected:
    bool m_blockChanged;
};


class BindSupportInfo
{
public:
    QStringList dataType;
};

class JZNodeQtBind
{
public:
    static BindSupportInfo BindSupport(QString className);
    
    static bool bind(QWidget *w,QVariant *v);
    static void unbind(QWidget *w);
    static bool uiToData(QWidget *w, QVariant *v);
    static bool dataToUi(QVariant *v, QWidget *w);
};


#endif // !JZNODE_QT_BIND_H_
