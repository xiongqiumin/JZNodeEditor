#ifndef JZ_PROPERTY_DIALOG_H_
#define JZ_PROPERTY_DIALOG_H_

#include "JZBaseDialog.h"
#include "jzWidgets/JZPropertyEditor.h"

class JZPropertyDialog : public JZBaseDialog
{
    Q_OBJECT

public:
    JZPropertyDialog(QWidget *parent = nullptr);
    void makeUniqueName(QString pre,QStringList nameList);

protected slots:
    void onPropTypeChanged(JZProperty * prop, const QVariant &v);

protected:
    void addPage(int type, QList<JZProperty*> propList);
    void switchPage(int page);
    bool isUniqueName();

    QStringList m_nameList;
    JZProperty *m_nameProp;
    JZProperty *m_typeProp;
    JZPropertyEditor *m_editor;
    QMap<int, QList<JZProperty*>> m_propType;
};


#endif