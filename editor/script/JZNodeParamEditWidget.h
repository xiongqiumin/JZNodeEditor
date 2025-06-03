#ifndef JZNODE_PARAM_EDIT_WIDGET_H_
#define JZNODE_PARAM_EDIT_WIDGET_H_

#include <QLabel>
#include <QToolButton>
#include <QLineEdit>
#include "JZScriptEnvironment.h"

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

//JZParamEditInfo
class JZParamEditInfo
{
public:
    enum{
        Edit_none,
        Edit_normal,
        Edit_bool,
        Edit_int,
        Edit_double,
        Edit_byteArray,
        Edit_enum,
        Edit_flag,
        Edit_file,
        Edit_dir,
    };

    static JZParamEditInfo createEnum(QStringList list);
    static JZParamEditInfo createType(const JZScriptEnvironment *env,QString type);

    JZParamEditInfo();

    int type;
    QVariant min,max;
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
    QWidget *m_editWidget;
};


#endif