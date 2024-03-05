#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCompleter>
#include "JZNodeObject.h"
#include "JZNodeParamWidget.h"
#include "JZNodeType.h"

//JZNodeParamTypeWidget
JZNodeParamTypeWidget::JZNodeParamTypeWidget(QWidget *parent)
    :QComboBox(parent)
{
    setEditable(true);
    
    QStringList type_list;
    type_list << "bool" << "int" << "double" << "string";
    type_list << JZNodeObjectManager::instance()->getClassList();
    type_list << JZNodeObjectManager::instance()->getEnumList();
    addItems(type_list);    

    QCompleter *comp = new QCompleter(type_list, this);
    comp->setCaseSensitivity(Qt::CaseInsensitive);
    setCompleter(comp);    
}

QString JZNodeParamTypeWidget::type()
{
    return currentText();
}

void JZNodeParamTypeWidget::setType(QString type)
{
    setEditText(type);
    setCurrentText(type);
}

//JZNodeParamValueWidget
JZNodeParamValueWidget::JZNodeParamValueWidget(QWidget *parent)
    :QWidget(parent)
{
    m_widget = nullptr;
    m_dataType = Type_none;
    
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);    
}

JZNodeParamValueWidget::~JZNodeParamValueWidget()
{

}

void JZNodeParamValueWidget::clearWidget()
{
    if (m_widget)
    {
        delete m_widget;
        m_widget = nullptr;
    }
}

void JZNodeParamValueWidget::onValueChanged()
{
    emit sigValueChanged(value());
}

void JZNodeParamValueWidget::setDataType(const QList<int> &type_list)
{
    int up_type = JZNodeType::upType(type_list);
    setDataType(JZNodeType::typeToName(up_type));
}

void JZNodeParamValueWidget::setDataType(QString type)
{        
    int dataType = JZNodeType::nameToType(type);
    if (dataType == m_dataType)
        return;
    
    clearWidget();
    m_dataType = dataType;          

    if (m_dataType == Type_bool || JZNodeType::isEnum(m_dataType))
    {        
        QComboBox *box = new QComboBox();
        if (m_dataType == Type_bool)
        {
            box->addItem("true", true);
            box->addItem("false", false);
        }
        else
        {
            auto meta = JZNodeObjectManager::instance()->enumMeta(m_dataType);
            for (int i = 0; i < meta->count(); i++)
                box->addItem(meta->key(i), meta->value(i));
        }        
        box->connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(onValueChanged()));        
        m_widget = box;
    }    
    else
    {        
        QLineEdit *edit = new QLineEdit();
        edit->connect(edit, SIGNAL(editingFinished()), this, SLOT(onValueChanged()));
        m_widget = edit;        
    }

    if (m_dataType == Type_none)
        m_widget->setEnabled(false);

    layout()->addWidget(m_widget);
}

QString JZNodeParamValueWidget::value() const
{
    if (!m_widget)
        return QString();

    QString text;
    if (m_widget->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(m_widget);
        text = box->currentText();
    }
    else if (m_widget->inherits("QLineEdit"))
    {        
        QLineEdit *edit = qobject_cast<QLineEdit*>(m_widget);
        if (m_dataType == Type_string)
            text = JZNodeType::storgeString(edit->text());
        else
            text = edit->text();
    }

    return text;
}

void JZNodeParamValueWidget::setValue(const QString &value)
{
    if (!m_widget)
        return;
    
    if (m_widget->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(m_widget);
        box->setCurrentText(value);
    }
    else if (m_widget->inherits("QLineEdit"))
    {        
        QLineEdit *edit = qobject_cast<QLineEdit*>(m_widget);
        if (m_dataType == Type_string)
            edit->setText(JZNodeType::dispString(value));
        else
            edit->setText(value);
    }
}