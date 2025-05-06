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

class JZParamEditInfo
{
public:
    enum{
        Edit_normal,
        Edit_enum,
        Edit_flag,
        Edit_file,
    };

    static JZParamEditInfo createEnum(QStringList list);

    JZParamEditInfo();

    int type;
    QString fileFilter;
    QStringList enumList;
};


//JZNodeParamValueWidget
class JZNodeParamValueWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamValueWidget();

    void init(const JZParamEditInfo &edit);

    void setValue(QString type);
    QString value();

signals:
    void sigEditFinish();

protected:
    bool eventFilter(QObject *object, QEvent *event);

    QWidget *m_editWidget;
};


#endif