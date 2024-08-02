#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCompleter>
#include <QDebug>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QApplication>
#include <QCheckBox>
#include "JZNodeObject.h"
#include "JZNodeParamWidget.h"
#include "JZNodeType.h"
#include "JZNodeFlagEditDialog.h"
#include "JZNodeTypeHelper.h"

//JZNodeFlagEditWidget
JZNodeFlagEditWidget::JZNodeFlagEditWidget(QWidget *parent)
    :QWidget(parent)
{    
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(1);

    m_line = new QLineEdit();
    m_line->setReadOnly(true);    
    QPushButton *btn = new QPushButton();
    connect(btn, &QPushButton::clicked, this , &JZNodeFlagEditWidget::onSettingClicked);
    l->addWidget(m_line);
    l->addWidget(btn);
    setLayout(l);    
}

JZNodeFlagEditWidget::~JZNodeFlagEditWidget()
{
}

void JZNodeFlagEditWidget::onSettingClicked()
{
    QString value = m_line->text();
    QString new_value;
    
    JZNodeFlagEditDialog dlg(this);
    dlg.init(m_flagType);
    dlg.setFlag(value);
    if (dlg.exec() != QDialog::Accepted)
        return;

    new_value = dlg.flag();   

    if (new_value != value)
    {
        setValue(new_value);
        emit sigValueChanged();
    }
}

void JZNodeFlagEditWidget::setFlagType(QString flag)
{
    m_flagType = flag;
}

QString JZNodeFlagEditWidget::flagType()
{
    return m_flagType;
}

QString JZNodeFlagEditWidget::value()
{
    return m_line->text();
}

void JZNodeFlagEditWidget::setValue(QString text)
{
    m_line->setText(text);
    m_line->setToolTip(text);
    m_line->setCursorPosition(0);
}

//JZNodeParamTypeWidget
JZNodeParamTypeWidget::JZNodeParamTypeWidget(QWidget *parent)
    :QComboBox(parent)
{
    setEditable(true);
    
    QStringList type_list;
    type_list << "bool" << "int" << "double" << "QString";
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

//JZNodePinWidget
JZNodePinWidget::JZNodePinWidget(QWidget *parent)
    :QWidget(parent)
{
}

JZNodePinWidget::~JZNodePinWidget()
{
}

QString JZNodePinWidget::value() const
{
    return QString();
}

void JZNodePinWidget::setValue(const QString &)
{
}

//JZNodePinButtonWidget
JZNodePinButtonWidget::JZNodePinButtonWidget()
{
    QVBoxLayout *lay = new QVBoxLayout();
    lay->setContentsMargins(0, 0, 0, 0);
    
    m_btn = new QPushButton();    
    lay->addWidget(m_btn);
    this->setLayout(lay);
    m_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

JZNodePinButtonWidget::~JZNodePinButtonWidget()
{

}

QPushButton *JZNodePinButtonWidget::button()
{
    return m_btn;
}

//JZNodeParamValueWidget
JZNodeParamValueWidget::JZNodeParamValueWidget(QWidget *parent)
    :JZNodePinWidget(parent)
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

QString JZNodeParamValueWidget::getWidgetType(int data_type)
{
    QString type;
    if (data_type == Type_bool)
        type = "QComboBox";
    else if(data_type == Type_hookEnable)
        type = "QCheckBox";
    else if (JZNodeType::isEnum(data_type))
    {
        auto meta = JZNodeObjectManager::instance()->enumMeta(data_type);
        if (meta->isFlag())
            type = "JZNodeFlagEditWidget";
        else
            type = "QComboBox";         
    }
    else
    {
        type = "QLineEdit";        
    }
    return type;
}

void JZNodeParamValueWidget::createWidget()
{    
    clearWidget();

    QString widget = m_widgetType;
    if (widget == "QCheckBox")
    {
        QCheckBox *box = new QCheckBox();
        box->connect(box, &QCheckBox::stateChanged, this, &JZNodeParamValueWidget::onValueChanged);
        connect(box, &QCheckBox::toggled, [box] {
            box->setText(box->isChecked() ? "true" : "false");
        });
        m_widget = box;
    }
    else if (widget == "QComboBox")
    {
        QComboBox *box = new QComboBox();        
        box->setEditable(true);

        auto line = box->findChild<QLineEdit*>();
        connect(line, &QLineEdit::editingFinished, this, &JZNodeParamValueWidget::onValueChanged);
        connect(line, &QLineEdit::returnPressed, this, &JZNodeParamValueWidget::onValueChanged);
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
    else if (widget == "JZNodeFlagEditWidget")
    {        
        JZNodeFlagEditWidget *edit = new JZNodeFlagEditWidget();        
        edit->connect(edit, &JZNodeFlagEditWidget::sigValueChanged, this, &JZNodeParamValueWidget::onValueChanged);
        m_widget = edit;
    }
    else
    {
        Q_ASSERT(0);
    }
    m_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    setFocusProxy(m_widget);
    layout()->addWidget(m_widget);        

    //init value
    m_widget->blockSignals(true);    
    if (m_widget->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(m_widget);
        box->clear();

        if (m_dataType == Type_bool)
        {
            box->addItem("true");
            box->addItem("false");

            box->setCompleter(JZNodeTypeHelper::instance()->boolCompleter());
        }
        else if(m_dataType == Type_int)
        {

        }
        else
        {
            auto meta = JZNodeObjectManager::instance()->enumMeta(m_dataType);
            Q_ASSERT(!meta->isFlag());
            
            if (meta->count() < 24)
            {
                for (int i = 0; i < meta->count(); i++)
                    box->addItem(meta->key(i), meta->value(i));
            }

            box->setCompleter(JZNodeTypeHelper::instance()->enumCompleter(m_dataType));
        }
    }
    else if (m_widget->inherits("QLineEdit"))
    {
    }
    else if (m_widget->inherits("QPlainTextEdit"))
    {
    }
    else if (m_widget->inherits("JZNodeFlagEditWidget"))
    {
        JZNodeFlagEditWidget *edit = qobject_cast<JZNodeFlagEditWidget*>(m_widget);        
        QString type_text = JZNodeType::typeToName(m_dataType);
        edit->setFlagType(type_text);
    }    
    m_widget->blockSignals(false);    
}

void JZNodeParamValueWidget::onValueChanged()
{
    clearFocus();
    emit sigValueChanged(value());    
}

void JZNodeParamValueWidget::initWidget(int data_type, QString widget_type)
{
    m_dataType = data_type;
    if(widget_type.isEmpty())
        m_widgetType = getWidgetType(m_dataType);
    else
        m_widgetType = widget_type;

    createWidget();
}

QWidget *JZNodeParamValueWidget::widget()
{
    return m_widget;
}

QString JZNodeParamValueWidget::value() const
{
    if (!m_widget)
        return QString();

    QString text;
    if (m_widget->inherits("QCheckBox"))
    {
        QCheckBox *box = qobject_cast<QCheckBox*>(m_widget);
        text = box->text();
    }
    else if (m_widget->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(m_widget);
        if (m_dataType == Type_int)
            text = QString::number(box->currentIndex());
        else
            text = box->currentText();
    }
    else if (m_widget->inherits("QLineEdit"))
    {        
        QLineEdit *edit = qobject_cast<QLineEdit*>(m_widget);
        text = edit->text();
    }
    else if (m_widget->inherits("QPlainTextEdit"))
    {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(m_widget);
        text = edit->toPlainText();
    }
    else if (m_widget->inherits("JZNodeFlagEditWidget"))
    {
        auto *edit = qobject_cast<JZNodeFlagEditWidget*>(m_widget);
        text = edit->value();
    }
    else
    {
        Q_ASSERT(0);
    }

    return text;
}

void JZNodeParamValueWidget::setValue(const QString &value)
{
    if (!m_widget)
        return;
    
    if (m_widget->inherits("QCheckBox"))
    {
        QCheckBox *box = qobject_cast<QCheckBox*>(m_widget);
        bool flag = value == "true";
        box->setChecked(flag);
        box->setText(flag? "true":"false");
    }
    else if (m_widget->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(m_widget);
        if(m_dataType == Type_int)
            box->setCurrentIndex(value.toInt());
        else
            box->setCurrentText(value);
    }
    else if (m_widget->inherits("QLineEdit"))
    {        
        QLineEdit *edit = qobject_cast<QLineEdit*>(m_widget);
        edit->setText(value);
        edit->setCursorPosition(0);
        edit->setToolTip(value);
    }
    else if (m_widget->inherits("QPlainTextEdit"))
    {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(m_widget);
        edit->setPlainText(value);
    }
    else if (m_widget->inherits("JZNodeFlagEditWidget"))
    {
        auto *edit = qobject_cast<JZNodeFlagEditWidget*>(m_widget);
        edit->setValue(value);
    }
    else
    {
        Q_ASSERT(0);
    }
}