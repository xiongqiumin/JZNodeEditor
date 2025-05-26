#include "JZPropertyDialog.h"

//JZPropertyDialog
JZPropertyDialog::JZPropertyDialog(QWidget *parent)
    : JZBaseDialog(parent)
{
    m_editor = new JZPropertyEditor();
    setCentralWidget(m_editor);
    m_typeProp = nullptr;

    resize(300, 400);
}

void JZPropertyDialog::addPage(int type, QList<JZProperty*> propList)
{
    m_propType[type] = propList;
}

void JZPropertyDialog::switchPage(int page)
{
    //hide
    auto it = m_propType.begin();
    while (it != m_propType.end())
    {
        if (it.key() != page)
        {
            auto &list = it.value();
            for (int i = 0; i < list.size(); i++)
                list[i]->setVisible(false);
        }
        it++;
    }

    //show
    it = m_propType.find(page);
    auto &show_list = it.value();
    for (int i = 0; i < show_list.size(); i++)
        show_list[i]->setVisible(true);
}

void JZPropertyDialog::onPropChanged(JZProperty * prop, const QVariant &v)
{
    if (prop != m_typeProp)
        return;

    int page = v.toInt();
    switchPage(page);
}