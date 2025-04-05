#ifndef JZNODE_PARAM_EDIT_WIDGET_H_
#define JZNODE_PARAM_EDIT_WIDGET_H_

#include <QLabel>
#include <QToolButton>


//JZNodeParamEditWidget
class JZNodeParamEditWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamEditWidget();
    virtual ~JZNodeParamEditWidget();
    
    virtual QString value() const = 0;
    virtual void setValue(const QString &text) = 0;

signals:
    void sigValueChanged();
};

//JZNodeParamPopupWidget
class JZNodeParamPopupWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamPopupWidget(QWidget *parent = nullptr);
    ~JZNodeParamPopupWidget();

    QString value();
    void setValue(QString text);

signals:
    void sigSettingClicked();

protected:
    QLineEdit *m_line;
};

class JZNodeParamTypeWidget : public QComboBox
{
    Q_OBJECT

public:
    JZNodeParamTypeWidget(QWidget *parent = nullptr);

    void init(JZNodeObjectManager *inst);
    QString type();
    void setType(QString type);

signals:
    void sigTypeChanged();

protected:
};

//ItemFocusEventFilter
class ItemFocusEventFilter : public QObject
{
public:
    ItemFocusEventFilter(QObject *parent);
    virtual bool eventFilter(QObject *object, QEvent *event) override;
};

//JZNodeParamValueWidget
class JZNodeParamValueWidget : public QWidget
{
    Q_OBJECT

public:
    JZNodeParamValueWidget();
    ~JZNodeParamValueWidget();

    void initWidget(int data_type, QString widget_type = QString());
    QWidget *focusWidget();

    QString value() const;
    void setValue(const QString &value);

signals:
    void sigValueChanged();

protected:
    QString getWidgetType(int data_type);
    void createWidget();
    void clearWidget();

    QWidget *m_widget;

    int m_dataType;
    QString m_widgetType;
};

//JZNodeImageEditWidget
class JZNodeImageEditWidget : public JZNodeParamEditWidget
{
    Q_OBJECT

public:
    JZNodeImageEditWidget();
    virtual ~JZNodeImageEditWidget();
    
    virtual QString value() const override;
    virtual void setValue(const QString &text) override;

signals:
    void sigValueChanged();

protected slots:
    void onFileActionActivated();

protected:
    QLabel *m_pixmapLabel;
    QLabel *m_pathLabel;
    QToolButton *m_button;
    QString m_path;
};


#endif