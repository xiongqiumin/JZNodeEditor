#ifndef JZ_PROPERTY_DIALOG_H_
#define JZ_PROPERTY_DIALOG_H_

#include "JZBaseDialog.h"
#include "jzWidgets/JZPropertyEditor.h"

class JZPropertyDialog : public JZBaseDialog
{
    Q_OBJECT

public:
    JZPropertyDialog(QWidget *parent = nullptr);

    protected slots:
    void onPropChanged(JZProperty * prop, const QVariant &v);

protected:
    void addPage(int type, QList<JZProperty*> propList);
    void switchPage(int page);

    JZProperty *m_typeProp;
    JZPropertyEditor *m_editor;
    QMap<int, QList<JZProperty*>> m_propType;
};


#endif