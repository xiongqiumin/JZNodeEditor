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


class JZNodeQtBind
{
public:
    static bool isBindSupport(QWidget *w, int type);
    static bool uiToData(QWidget *w, QVariant &v);
    static bool dataToUi(const QVariant &v,QWidget *w);   
    
    static bool bind(QWidget *w,QVariant *v);
    static void unbind(QWidget *w);
};


#endif // !JZNODE_QT_BIND_H_
