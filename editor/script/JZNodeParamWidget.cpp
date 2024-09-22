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

//JZNodeParamEditWidget
JZNodeParamEditWidget::JZNodeParamEditWidget()
{
    setFocusPolicy(Qt::StrongFocus);
}

JZNodeParamEditWidget::~JZNodeParamEditWidget()
{
}

void JZNodeParamEditWidget::init()
{

}

//JZNodeParamDiaplayWidget
JZNodeParamDiaplayWidget::JZNodeParamDiaplayWidget()
{
}

JZNodeParamDiaplayWidget::~JZNodeParamDiaplayWidget()
{
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
    else if (JZNodeParamWidgetManager::instance()->hasEditWidget(data_type))
    {
        type = "JZNodeParamEditWidget";
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
    else if (widget == "JZNodeParamEditWidget")
    {
        auto edit = JZNodeParamWidgetManager::instance()->createEditWidget(m_dataType);
        edit->connect(edit, &JZNodeParamEditWidget::sigValueChanged, this, &JZNodeParamValueWidget::onValueChanged);
        m_widget = edit;
    }
    else
    {
        Q_ASSERT(0);
    }
    m_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);    
    layout()->addWidget(m_widget);        
    setFocusProxy(m_widget);
    qDebug() << "m_widget" << m_widget->focusPolicy();

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
    else if (widget == "JZNodeParamEditWidget")
    {
        JZNodeParamEditWidget *edit = qobject_cast<JZNodeParamEditWidget*>(m_widget);
        edit->init();
    }

    m_widget->blockSignals(false);    
}

void JZNodeParamValueWidget::onValueChanged()
{    
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

//JZNodeDisplayWidget
JZNodeDisplayWidget::JZNodeDisplayWidget()
{
    m_widget = nullptr;

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);        
}

void JZNodeDisplayWidget::init()
{
    createWidget();
}

void JZNodeDisplayWidget::createWidget()
{
    QString cur_type, need_type;
    if (m_widget)
        cur_type = m_widget->metaObject()->className();

    need_type = "QLineEdit";    
    if (m_node)
    {
        auto file = m_node->file();
        auto list = file->getConnectPin(m_node->id(), m_pin);
        if (list.size() == 1)
        {
            auto line = file->getConnect(list[0]);
            auto type_list = file->getPin(line->from)->dataTypeId();
            if (type_list.size() == 1 && type_list[0] == Type_image)
                need_type = "QImageLabel";
        }
    }
    if (cur_type == need_type)
        return;    

    if (m_widget)
        delete m_widget;

    if (need_type == "QLineEdit")
    {
        auto line = new QLineEdit();        
        line->setReadOnly(true);        
        m_widget = line;
        resize(120, 24);
    }
    else if (need_type == "QImageLabel")
    {
        auto image = new QImageLabel();        
        m_widget = image;
        resize(100, 100);
    }
    m_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    layout()->addWidget(m_widget);
}

void JZNodeDisplayWidget::updateWidget()
{
    createWidget();
    emit sigSizeChanged(size());
}

void JZNodeDisplayWidget::setRuntimeValue(const JZNodeDebugParamValue &value)
{
    if (m_widget->inherits("QLineEdit"))
    {
        auto line = qobject_cast<QLineEdit*>(m_widget);
        line->setText(value.value);
    }
    else if (m_widget->inherits("QImageLabel"))
    {
        auto image_label = qobject_cast<QImageLabel*>(m_widget);
        if (value.ptrValue && JZNodeType::variantType(*value.ptrValue) == Type_image)
        {
            QImage *image = JZObjectCast<QImage>(*value.ptrValue);
            image_label->setImage(*image);
        }
    }    
}

//JZNodeParamWidgetManager
JZNodeParamWidgetManager *JZNodeParamWidgetManager::instance()
{
    static JZNodeParamWidgetManager inst;
    return &inst;
}


JZNodeParamWidgetManager::JZNodeParamWidgetManager()
{
}

JZNodeParamWidgetManager::~JZNodeParamWidgetManager()
{
}

void JZNodeParamWidgetManager::registEditWidget(int edit_type, CreateParamEditFunc func)
{
    m_editFactory[edit_type] = func;
}

void JZNodeParamWidgetManager::registEditDelegate(int data_type, int edit_type, CreateParamFunc func)
{
    EditDelegate d;
    d.editType = edit_type;
    d.func = func;
    m_editDelegate[data_type] = d;
}

void JZNodeParamWidgetManager::registDisplayWidget(int data_type, CreateParamDiaplayFunc func)
{
    m_displayFactory[data_type] = func;
}

bool JZNodeParamWidgetManager::hasEditDelegate(int data_type)
{
    return m_editDelegate.contains(data_type);                                                                        
}

bool JZNodeParamWidgetManager::hasEditWidget(int data_type)
{
    return m_editFactory.contains(data_type);
}

bool JZNodeParamWidgetManager::hasDisplayWidget(int data_type)
{
    return m_displayFactory.contains(data_type);
}

int JZNodeParamWidgetManager::editDelegate(int data_type)
{
    if (!m_editDelegate.contains(data_type))
        return Type_none;

    return m_editDelegate[data_type].editType;
}

JZNodeParamEditWidget *JZNodeParamWidgetManager::createEditWidget(int data_type)
{
    return m_editFactory[data_type]();
}

QVariant JZNodeParamWidgetManager::createEditParam(int data_type,const QString &value)
{
    return m_editDelegate[data_type].func(value);
}

JZNodeParamDiaplayWidget *JZNodeParamWidgetManager::createDisplayWidget(int data_type)
{
    return m_displayFactory[data_type]();
}