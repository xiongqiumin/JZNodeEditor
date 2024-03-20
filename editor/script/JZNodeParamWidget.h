#ifndef JZNODE_PARAM_WIDGET_H_
#define JZNODE_PARAM_WIDGET_H_

#include <QWidget>
#include <QComboBox>

class JZNodeParamEditWidget : public QWidget
{
    Q_OBJECT

public:
    enum {
        Edit_none,
        Edit_flag,
        Edit_number_string,
    };

    JZNodeParamEditWidget(QWidget *parent = nullptr);

    void setType(int type);

    QString value();
    void setValue(QString text);

    void setExt(QString param);
    QString ext();

signals:
    void sigValueChanged();

protected slots:
    void onSettingClicked();

protected:
    QLineEdit *m_line;
    QString m_ext;
    int m_type;
};

class JZNodeParamTypeWidget : public QComboBox
{
    Q_OBJECT

public:
    JZNodeParamTypeWidget(QWidget *parent = nullptr);

    QString type();
    void setType(QString type);

signals:
    void sigTypeChanged();

protected:
};


class JZNodeParamValueWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamValueWidget(QWidget *parent = nullptr);
    ~JZNodeParamValueWidget();

    void setWidgetType(QString widget);
    void setDataType(const QList<int> &type_list);    

    QString value() const;
    void setValue(const QString &value);
    
    void setEditable(bool flag);

signals:
    void sigValueChanged(const QString &value);

protected slots:
    void onValueChanged();

protected:
    void createWidget(QString widget_type);    
    QString getWidgetType(int data_type);
    void clearWidget();    
    void initWidget();

    QWidget *m_widget;

    QList<int> m_dataType;
    QString m_widgetType;
};



#endif // !JZNODE_PARAM_WIDGET_H_
