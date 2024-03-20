#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCompleter>
#include <QDebug>
#include <QPushButton>
#include <QPlainTextEdit>
#include "JZNodeObject.h"
#include "JZNodeParamWidget.h"
#include "JZNodeType.h"
#include "JZNodeFlagEditDialog.h"

//JZNodeParamEditWidget
JZNodeParamEditWidget::JZNodeParamEditWidget(QWidget *parent)
    :QWidget(parent)
{    
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(1);
    m_line = new QLineEdit();
    m_line->setReadOnly(true);
    QPushButton *btn = new QPushButton();
    connect(btn, &QPushButton::clicked, this , &JZNodeParamEditWidget::onSettingClicked);
    l->addWidget(m_line);
    l->addWidget(btn);
    setLayout(l);    

    m_type = Edit_none;
}

void JZNodeParamEditWidget::onSettingClicked()
{
    QString value = m_line->text();
    QString new_value;

    if (m_type == Edit_flag)
    {
        JZNodeFlagEditDialog dlg(this);
        dlg.init(m_ext);
        dlg.setFlag(value);
        if (dlg.exec() != QDialog::Accepted)
            return;

        new_value = dlg.flag();
    }
    else
    {
        JZNodeNumberStringEditDialog dlg(this);
        dlg.setValue(value);
        if (dlg.exec() != QDialog::Accepted)
            return;

        new_value = dlg.value();
    }

    if (new_value != value)
    {
        setValue(new_value);
        emit sigValueChanged();
    }
}

void JZNodeParamEditWidget::setType(int type)
{
    m_type = type;
}

QString JZNodeParamEditWidget::value()
{
    return m_line->text();
}

void JZNodeParamEditWidget::setValue(QString text)
{
    m_line->setText(text);
    m_line->setToolTip(text);
    m_line->setCursorPosition(0);
}

void JZNodeParamEditWidget::setExt(QString ext)
{
    m_ext = ext;
}

QString JZNodeParamEditWidget::ext()
{
    return m_ext;
}

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

    auto line = findChild<QLineEdit*>();
    connect(line, &QLineEdit::editingFinished, this, &JZNodeParamTypeWidget::sigTypeChanged);
    connect(this,QOverload<int>::of(&JZNodeParamTypeWidget::currentIndexChanged), this, &JZNodeParamTypeWidget::sigTypeChanged);
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
    
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);    
}

JZNodeParamValueWidget::~JZNodeParamValueWidget()
{

}

void JZNodeParamValueWidget::setEditable(bool flag)
{
    m_widget->setVisible(flag);
}

void JZNodeParamValueWidget::clearWidget()
{
    if (m_widget)
    {
        delete m_widget;
        m_widgetType.clear();
        m_widget = nullptr;
    }
}

QString JZNodeParamValueWidget::getWidgetType(int data_type)
{
    QString type;
    if (data_type == Type_bool)
        type = "QComboBox";
    else if (JZNodeType::isEnum(data_type))
    {
        auto meta = JZNodeObjectManager::instance()->enumMeta(data_type);
        if (meta->isFlag())
            type = "JZNodeParamEditWidget";
        else
            type = "QComboBox";         
    }    
    else
    {
        type = "QLineEdit";        
    }
    return type;
}

void JZNodeParamValueWidget::createWidget(QString widget)
{    
    clearWidget();

    if (widget == "QComboBox")
    {
        QComboBox *box = new QComboBox();
        box->connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(onValueChanged()));
        m_widget = box;
    }
    else if (widget == "QPlainTextEdit")
    {
        QPlainTextEdit *edit = new QPlainTextEdit();
        edit->connect(edit, &QPlainTextEdit::textChanged, this, &JZNodeParamValueWidget::onValueChanged);
        m_widget = edit;
    }
    else if (widget == "QLineEdit")
    {
        QLineEdit *edit = new QLineEdit();
        edit->connect(edit, SIGNAL(editingFinished()), this, SLOT(onValueChanged()));
        m_widget = edit;
    }
    else if (widget == "JZNodeParamEditWidget")
    {        
        JZNodeParamEditWidget *edit = new JZNodeParamEditWidget();        
        edit->connect(edit, &JZNodeParamEditWidget::sigValueChanged, this, &JZNodeParamValueWidget::onValueChanged);
        m_widget = edit;
    }

    layout()->addWidget(m_widget);
}

void JZNodeParamValueWidget::initWidget()
{
    m_widget->blockSignals(true);
    int up_type = JZNodeType::upType(m_dataType);
    if (m_widget->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(m_widget);
        box->clear();

        if (up_type == Type_bool)
        {
            box->addItem("true");
            box->addItem("false");
        }
        else
        {
            auto meta = JZNodeObjectManager::instance()->enumMeta(up_type);
            Q_ASSERT(!meta->isFlag());

            for (int i = 0; i < meta->count(); i++)
                box->addItem(meta->key(i), meta->value(i));            
        }
    }
    else if (m_widget->inherits("QLineEdit"))
    {
    }
    else if (m_widget->inherits("QPlainTextEdit"))
    {
    }
    else if (m_widget->inherits("JZNodeParamEditWidget"))
    {
        JZNodeParamEditWidget *edit = qobject_cast<JZNodeParamEditWidget*>(m_widget);
        if (JZNodeType::isEnum(up_type))
        {
            QString type_text = JZNodeType::typeToName(up_type);            
            edit->setType(JZNodeParamEditWidget::Edit_flag);
            edit->setExt(type_text);
        }
        else
        {
            edit->setType(JZNodeParamEditWidget::Edit_number_string);
        }
    }    

    if (m_widget->inherits("QLineEdit"))  
        m_widget->setFixedWidth(80);    

    m_widget->blockSignals(false);    
}

void JZNodeParamValueWidget::onValueChanged()
{
    emit sigValueChanged(value());
}

void JZNodeParamValueWidget::setWidgetType(QString widget)
{
    m_widgetType = widget;
    createWidget(m_widgetType);
}
void JZNodeParamValueWidget::setDataType(const QList<int> &type_list)
{
    m_dataType = type_list;

    int up_type = JZNodeType::upType(type_list);              
    if (m_widgetType.isEmpty())
    {
        QString widget_type = getWidgetType(up_type);
        createWidget(widget_type);
    }
    initWidget();            
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
        if (m_dataType.contains(Type_string))
            text = JZNodeType::storgeString(edit->text());
        else
            text = edit->text();
    }
    else if (m_widget->inherits("QPlainTextEdit"))
    {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(m_widget);
        text = edit->toPlainText();
    }
    else if (m_widget->inherits("JZNodeParamEditWidget"))
    {
        auto *edit = qobject_cast<JZNodeParamEditWidget*>(m_widget);
        text = edit->value();
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
        if (m_dataType.contains(Type_string))
            edit->setText(JZNodeType::dispString(value));
        else
            edit->setText(value);
        edit->setCursorPosition(0);
        edit->setToolTip(value);
    }
    else if (m_widget->inherits("QPlainTextEdit"))
    {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(m_widget);
        edit->setPlainText(value);
    }
    else if (m_widget->inherits("JZNodeParamEditWidget"))
    {
        auto *edit = qobject_cast<JZNodeParamEditWidget*>(m_widget);
        edit->setValue(value);
    }
}