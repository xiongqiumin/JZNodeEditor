#ifndef JZNODE_QT_BIND_H_
#define JZNODE_QT_BIND_H_

#include <QWidget>
#include <JZNodeType.h>

class JZBodeQtBind
{
public:
    static bool isBindSupport(QWidget *w, const QVariant &v);
    static bool uiToData(QWidget *w, QVariant &v);
    static bool dataToUi(const QVariant &v,QWidget *w);   
    static bool bind(QWidget *w);
};


#endif // !JZNODE_QT_BIND_H_
