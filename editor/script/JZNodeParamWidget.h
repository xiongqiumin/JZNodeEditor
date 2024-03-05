#ifndef JZNODE_PARAM_WIDGET_H_
#define JZNODE_PARAM_WIDGET_H_

#include <QWidget>
#include <QComboBox>

class JZNodeParamTypeWidget : public QComboBox
{
public:
    JZNodeParamTypeWidget(QWidget *parent = nullptr);

    QString type();
    void setType(QString type);
};


class JZNodeParamValueWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamValueWidget(QWidget *parent = nullptr);
    ~JZNodeParamValueWidget();

    void setDataType(QString type);
    void setDataType(const QList<int> &type_list);

    QString value() const;
    void setValue(const QString &value);

signals:
    void sigValueChanged(const QString &value);

protected slots:
    void onValueChanged();

protected:
    void clearWidget();

    QWidget *m_widget;
    int m_dataType;
};



#endif // !JZNODE_PARAM_WIDGET_H_
