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

void JZNodeFlagEditDialog::init(QString flagName)
{    
    m_flag = flagName;
    auto meta = JZNodeObjectManager::instance()->enumMeta(flagName);
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
    auto meta = JZNodeObjectManager::instance()->enumMeta(m_flag);
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

//JZNodeNumberStringEditDialog
JZNodeNumberStringEditDialog::JZNodeNumberStringEditDialog(QWidget *parent)
    : JZBaseDialog(parent)
{
    QVBoxLayout *l = new QVBoxLayout();
    m_box = new QComboBox();
    m_box->addItem("int", Type_int);
    m_box->addItem("double", Type_double);
    m_box->addItem("string", Type_string);

    m_line = new QLineEdit();
    l->addWidget(m_box);
    l->addWidget(m_line);
    m_mainWidget->setLayout(l);
}

JZNodeNumberStringEditDialog::~JZNodeNumberStringEditDialog()
{
}

void JZNodeNumberStringEditDialog::setValue(QString value)
{
    int data_type = JZNodeType::stringType(value);
    int idx = m_box->findData(data_type);
    m_box->setCurrentIndex(idx);

    if (data_type == Type_string)
        m_line->setText(JZNodeType::dispString(value));
    else
        m_line->setText(value);
}

QString JZNodeNumberStringEditDialog::value()
{
    int data_type = m_box->currentData().toInt();
    if (data_type == Type_string)
        return JZNodeType::storgeString(m_line->text());
    else
        return m_line->text();
}

bool JZNodeNumberStringEditDialog::onOk()
{
    return true;
}