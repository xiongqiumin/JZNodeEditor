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
#include "JZNodePinWidget.h"
#include "JZNodeType.h"
#include "JZNodeFlagEditDialog.h"
#include "JZNodeTypeHelper.h"
#include "JZNodeParamEditWidget.h"
#include "JZNodeParamDelegate.h"
#include "JZNode.h"
#include "JZScriptEnvironment.h"

//JZNodePinPopupWidget
JZNodePinPopupWidget::JZNodePinPopupWidget(QWidget *parent)
    :QWidget(parent)
{    
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(1);

    m_line = new QLineEdit();
    m_line->setReadOnly(true);    
    QPushButton *btn = new QPushButton();
    connect(btn, &QPushButton::clicked, this , &JZNodePinPopupWidget::sigSettingClicked);
    l->addWidget(m_line);
    l->addWidget(btn);
    setLayout(l);    
}

JZNodePinPopupWidget::~JZNodePinPopupWidget()
{
}

QString JZNodePinPopupWidget::value()
{
    return m_line->text();
}

void JZNodePinPopupWidget::setValue(QString text)
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
}

void JZNodeParamTypeWidget::init(JZNodeObjectManager *inst)
{
    QStringList type_list;
    type_list << "bool" << "int" << "double" << "string";
    type_list << inst->getClassList();
    type_list << inst->getEnumList();
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
    m_node = nullptr;
    m_pin = -1;
}

JZNodePinWidget::~JZNodePinWidget()
{
}

void JZNodePinWidget::init()
{

}

void JZNodePinWidget::init(JZNode *node, int pin_id)
{
    m_node = node;
    m_pin = pin_id;
    init();
}

QString JZNodePinWidget::value() const
{
    return QString();
}

void JZNodePinWidget::setValue(const QString &)
{
}

void JZNodePinWidget::updateWidget()
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

//JZNodePinValueWidget
JZNodePinValueWidget::JZNodePinValueWidget(QWidget *parent)
    :JZNodePinWidget(parent)
{
    m_widget = nullptr;
    m_dataType = Type_none;
    
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);    
}

JZNodePinValueWidget::~JZNodePinValueWidget()
{

}

void JZNodePinValueWidget::clearWidget()
{
    if (m_widget)
    {
        delete m_widget;
        m_widget = nullptr;
    }
}

QString JZNodePinValueWidget::getWidgetType(int data_type)
{
    QString type;
    if (data_type == Type_bool)
        type = "QComboBox";
    else if(data_type == Type_hookEnable)
        type = "QCheckBox";    
    else if (JZNodeType::isEnum(data_type))
    {
        auto obj_inst = m_node->environment()->objectManager();
        auto meta = obj_inst->enumMeta(data_type);
        if (meta->isFlag())
            type = "JZNodePinPopupWidget";
        else
            type = "QComboBox";         
    }
    else if (JZNodeParamDelegateManager::instance()->editFunction(data_type))
    {
        type = "JZNodeParamEditWidget";
    }
    else
    {
        type = "QLineEdit";        
    }
    return type;
}

void JZNodePinValueWidget::createWidget()
{    
    clearWidget();

    auto obj_inst = m_node->environment()->objectManager();
    QString widget = m_widgetType;
    if (widget == "QCheckBox")
    {
        QCheckBox *box = new QCheckBox();
        box->connect(box, &QCheckBox::stateChanged, this, &JZNodePinValueWidget::onValueChanged);
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
        connect(line, &QLineEdit::editingFinished, this, &JZNodePinValueWidget::onValueChanged);
        connect(line, &QLineEdit::returnPressed, this, &JZNodePinValueWidget::onValueChanged);
        box->connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(onValueChanged()));        
        m_widget = box;
    }
    else if (widget == "QPlainTextEdit")
    {
        QPlainTextEdit *edit = new QPlainTextEdit();
        edit->connect(edit, &QPlainTextEdit::textChanged, this, &JZNodePinValueWidget::onValueChanged);
        m_widget = edit;
    }
    else if (widget == "QLineEdit")
    {
        QLineEdit *edit = new QLineEdit();
        edit->connect(edit, SIGNAL(editingFinished()), this, SLOT(onValueChanged()));
        m_widget = edit;
    }
    else if (widget == "JZNodePinPopupWidget")
    {        
        JZNodePinPopupWidget *edit = new JZNodePinPopupWidget();
        /*
        edit->connect(edit, &JZNodePinPopupWidget::sigSettingClicked, this, [this] {            
            QString value;
            QString new_value;

            auto meta = m_node->environment()->objectManager()->enumMeta(value);
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
        });
        */
        m_widget = edit;
    }
    else if (widget == "JZNodeParamEditWidget")
    {
        auto edit = JZNodeParamDelegateManager::instance()->editFunction(m_dataType)();
        edit->connect(edit, &JZNodeParamEditWidget::sigValueChanged, this, &JZNodePinValueWidget::onValueChanged);
        m_widget = edit;
    }
    else
    {
        Q_ASSERT(0);
    }
    m_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);    
    layout()->addWidget(m_widget);        
    setFocusProxy(m_widget);    

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
            auto meta = obj_inst->enumMeta(m_dataType);
            Q_ASSERT(!meta->isFlag());
            
            for (int i = 0; i < meta->count(); i++)
                box->addItem(meta->key(i), meta->value(i));            
            box->setCompleter(JZNodeTypeHelper::instance()->enumCompleter(m_dataType));
        }
    }
    else if (m_widget->inherits("QLineEdit"))
    {
    }
    else if (m_widget->inherits("QPlainTextEdit"))
    {
    }
    else if (m_widget->inherits("JZNodePinPopupWidget"))
    {
        JZNodePinPopupWidget *edit = qobject_cast<JZNodePinPopupWidget*>(m_widget);        
        QString type_text = editorEnvironment()->typeToName(m_dataType);
        //edit->setFlagType(type_text);
    }    
    else if (widget == "JZNodeParamEditWidget")
    {
        JZNodeParamEditWidget *edit = qobject_cast<JZNodeParamEditWidget*>(m_widget);
        edit->init();
    }

    m_widget->blockSignals(false);    
}

void JZNodePinValueWidget::onValueChanged()
{    
    emit sigValueChanged(value());    
}

void JZNodePinValueWidget::initWidget(int data_type, QString widget_type)
{
    m_dataType = data_type;
    if(widget_type.isEmpty())
        m_widgetType = getWidgetType(m_dataType);
    else
        m_widgetType = widget_type;

    createWidget();
}

QWidget *JZNodePinValueWidget::focusWidget()
{
    return m_widget;
}

QString JZNodePinValueWidget::value() const
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
    else if (m_widget->inherits("JZNodePinPopupWidget"))
    {
        auto *edit = qobject_cast<JZNodePinPopupWidget*>(m_widget);
        text = edit->value();
    }
    else if (m_widget->inherits("JZNodeParamEditWidget"))
    {
        JZNodeParamEditWidget *edit = qobject_cast<JZNodeParamEditWidget*>(m_widget);
        text = edit->value();
    }
    else
    {
        Q_ASSERT(0);
    }

    return text;
}

void JZNodePinValueWidget::setValue(const QString &value)
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
    else if (m_widget->inherits("JZNodePinPopupWidget"))
    {
        auto *edit = qobject_cast<JZNodePinPopupWidget*>(m_widget);
        edit->setValue(value);
    }
    else if (m_widget->inherits("JZNodeParamEditWidget"))
    {
        JZNodeParamEditWidget *edit = qobject_cast<JZNodeParamEditWidget*>(m_widget);
        edit->setValue(value);
    }
    else
    {
        Q_ASSERT(0);
    }
}

//JZNodePinDisplayWidget
JZNodePinDisplayWidget::JZNodePinDisplayWidget()
{
    m_widget = nullptr;

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);        
    m_dataType = Type_none;
}

void JZNodePinDisplayWidget::init()
{
    createWidget();
}

void JZNodePinDisplayWidget::createWidget()
{    
    auto env = editorEnvironment();
    int need_type = Type_none;
    if (m_node)
    {
        auto file = m_node->file();
        auto list = file->getConnectPin(m_node->id(), m_pin);
        if (list.size() == 1)
        {
            auto line = file->getConnect(list[0]);
            auto type_list = env->nameToTypeList(file->getPin(line->from)->dataType());
            if (type_list.size() == 1 && type_list[0] == Type_image)
                need_type = Type_image;
        }
    }
    if (m_dataType == need_type)
        return;    

    if (m_widget)
        delete m_widget;

    m_dataType = need_type;
    auto func = JZNodeParamDelegateManager::instance()->displayFunction(m_dataType);

    if (func)
    {        
        m_widget = func();        
        resize(m_widget->size());
    }
    else
    {        
        auto line = new QLineEdit();
        line->setReadOnly(true);
        m_widget = line;
        resize(120, 24);
    }    
    m_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    layout()->addWidget(m_widget);
}

void JZNodePinDisplayWidget::updateWidget()
{
    createWidget();
    emit sigSizeChanged(size());
}

void JZNodePinDisplayWidget::setRuntimeValue(const JZNodeDebugParamValue &value)
{
    if (m_widget->inherits("QLineEdit"))
    {
        auto line = qobject_cast<QLineEdit*>(m_widget);
        line->setText(value.value);
    }
    else if (m_widget->inherits("JZNodeParamDisplayWidget"))
    {
        auto display = qobject_cast<JZNodeParamDisplayWidget*>(m_widget);
        display->setRuntimeValue(value);
    }    
}