#ifndef JZNODE_PARAM_EDIT_WIDGET_H_
#define JZNODE_PARAM_EDIT_WIDGET_H_

#include <QLabel>
#include <QToolButton>
#include "jzWidgets/JZPropertyWidget.h"

//JZNodeParamTypeWidget
class JZNodeParamTypeWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamTypeWidget();

    void setType(QString type);
    QString type();

protected:
    QLineEdit *m_lineEdit;
};

//JZNodeParamValueWidget
class JZNodeParamValueWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamValueWidget();

    void initWidget(int type);
    void setValue(QString type);
    QString value();

signals:
    void sigEditFinish();

protected:
    bool eventFilter(QObject *object, QEvent *event);

    QLineEdit *m_lineEdit;
};


#endif