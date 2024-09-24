#include <QMessageBox>
#include <QFontMetrics>
#include <QDebug>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include "JZNodeFlagEditDialog.h"
#include "JZNodeObject.h"

//JZNodeFlagEditDialog
JZNodeFlagEditDialog::JZNodeFlagEditDialog(QWidget *parent)
    : JZBaseDialog(parent)
{    
}

JZNodeFlagEditDialog::~JZNodeFlagEditDialog()
{
}

void JZNodeFlagEditDialog::init(JZNodeEnumDefine *meta)
{    
    m_enumMeta = meta;    
    int n = meta->count();
    
    QVBoxLayout *l = new QVBoxLayout();
    for (int i = 0; i < n; i++)
    {
        QCheckBox *box = new QCheckBox(meta->key(i));
        l->addWidget(box);

        m_boxList.push_back(box);
    }
    m_mainWidget->setLayout(l);
    setFlag(meta->defaultKey());
}

void JZNodeFlagEditDialog::setFlag(QString flag)
{    
    QStringList list = flag.split("|");
    for (int i = 0; i < m_boxList.size(); i++)
    {
        if (list.contains(m_boxList[i]->text()))
            m_boxList[i]->setChecked(true);
    }
}

QString JZNodeFlagEditDialog::flag()
{
    return m_flagKey;
}

bool JZNodeFlagEditDialog::onOk()
{
    auto meta = m_enumMeta;
    QStringList keyList;
    for (int i = 0; i < m_boxList.size(); i++)
    {
        if (m_boxList[i]->isChecked())
            keyList << m_boxList[i]->text();
    }
    if (keyList.size() != 0)
        m_flagKey = keyList.join("|");
    else
        m_flagKey = meta->defaultKey();

    return true;
}