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

    virtual void init();
    virtual QString value() = 0;
    virtual void setValue(const QString &text) = 0;

signals:
    void sigValueChanged();
};

//JZNodeImageEditWidget
class JZNodeImageEditWidget : public JZNodeParamEditWidget
{
    Q_OBJECT

public:
    JZNodeImageEditWidget();
    virtual ~JZNodeImageEditWidget();
    
    virtual QString value() override;
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