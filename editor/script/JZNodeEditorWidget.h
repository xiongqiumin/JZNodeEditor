#ifndef JZNODE_EDITOR_WIDGET_H_
#define JZNODE_EDITOR_WIDGET_H_

#include <QLabel>
#include <QToolButton>
#include "JZNodeParamWidget.h"


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


void InitEditorWidget();


#endif