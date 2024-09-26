#include <QHBoxLayout>
#include <QFileDialog>
#include <QIcon>
#include <QComboBox>
#include <QLineEdit>
#include <QCompleter>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QApplication>
#include "JZEditorGlobal.h"
#include "JZNodeParamEditWidget.h"
#include "JZNodeTypeHelper.h"
#include "JZNodeFlagEditDialog.h"

//JZNodeParamEditWidget
JZNodeParamEditWidget::JZNodeParamEditWidget()
{
}

JZNodeParamEditWidget::~JZNodeParamEditWidget()
{
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
    connect(this, QOverload<int>::of(&JZNodeParamTypeWidget::currentIndexChanged), this, &JZNodeParamTypeWidget::sigTypeChanged);
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

//JZNodeParamPopupWidget
JZNodeParamPopupWidget::JZNodeParamPopupWidget(QWidget *parent)
    :QWidget(parent)
{
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(1);

    m_line = new QLineEdit();
    m_line->setReadOnly(true);
    QPushButton *btn = new QPushButton();
    connect(btn, &QPushButton::clicked, this, &JZNodeParamPopupWidget::sigSettingClicked);
    l->addWidget(m_line);
    l->addWidget(btn);
    setLayout(l);
}

JZNodeParamPopupWidget::~JZNodeParamPopupWidget()
{
}

QString JZNodeParamPopupWidget::value()
{
    return m_line->text();
}

void JZNodeParamPopupWidget::setValue(QString text)
{
    m_line->setText(text);
    m_line->setToolTip(text);
    m_line->setCursorPosition(0);
}


ItemFocusEventFilter::ItemFocusEventFilter(QObject *parent)
    :QObject(parent)
{
}

bool ItemFocusEventFilter::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::FocusAboutToChange:
    {
        QFocusEvent *fe = static_cast<QFocusEvent *>(event);
        if (fe->reason() == Qt::ActiveWindowFocusReason)
            return false;

        auto main = object->parent();
        while (main)
        {
            if (main->property("isEditor").toBool())
                break;
            main = main->parent();
        }

        // Forward focus events to editor because the QStyledItemDelegate relies on them                  
        QCoreApplication::sendEvent(main, event);
        break;
    }
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

//JZNodeParamValueWidget
JZNodeParamValueWidget::JZNodeParamValueWidget()    
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
    auto d = editorEnvironment()->editorManager()->delegate(m_dataType);

    QString type;
    if (data_type == Type_bool)
        type = "QComboBox";
    else if (data_type == Type_boolCheck)
        type = "QCheckBox";
    else if (JZNodeType::isEnum(data_type))
    {
        auto obj_inst = editorEnvironment()->objectManager();
        auto meta = obj_inst->enumMeta(data_type);
        if (meta->isFlag())
            type = "JZNodeParamPopupWidget";
        else
            type = "QComboBox";
    }
    else if (d && d->createEdit)
    {
        type = "JZNodeParamEditWidget";
    }
    else
    {
        type = "QLineEdit";
    }

    return type;
}

void JZNodeParamValueWidget::initWidget(int data_type, QString widget_type)
{
    m_dataType = data_type;
    if (widget_type.isEmpty())
        m_widgetType = getWidgetType(m_dataType);
    else
        m_widgetType = widget_type;

    createWidget();
}

void JZNodeParamValueWidget::createWidget()
{
    clearWidget();

    blockSignals(true);

    auto obj_inst = editorObjectManager();
    QString widget = m_widgetType;
    if (widget == "QCheckBox")
    {
        QCheckBox *box = new QCheckBox();
        box->connect(box, &QCheckBox::stateChanged, this, &JZNodeParamValueWidget::sigValueChanged);
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
        connect(line, &QLineEdit::editingFinished, this, &JZNodeParamValueWidget::sigValueChanged);
        connect(line, &QLineEdit::returnPressed, this, &JZNodeParamValueWidget::sigValueChanged);
        box->connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &JZNodeParamValueWidget::sigValueChanged);

        if (m_dataType == Type_bool)
        {
            box->addItem("true");
            box->addItem("false");

            box->setCompleter(JZNodeTypeHelper::instance()->boolCompleter());
        }
        else if (m_dataType == Type_int)
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
        m_widget = box;
    }
    else if (widget == "QPlainTextEdit")
    {
        QPlainTextEdit *edit = new QPlainTextEdit();        
        m_widget = edit;
    }
    else if (widget == "QLineEdit")
    {
        QLineEdit *edit = new QLineEdit();
        edit->connect(edit, &QLineEdit::editingFinished, this, &JZNodeParamValueWidget::sigValueChanged);
        edit->connect(edit, &QLineEdit::returnPressed, this, &JZNodeParamValueWidget::sigValueChanged);
        m_widget = edit;
    }
    else if (widget == "JZNodeParamPopupWidget")
    {
        JZNodeParamPopupWidget *edit = new JZNodeParamPopupWidget();      
        edit->connect(edit, &JZNodeParamPopupWidget::sigSettingClicked, this, [edit,this]
        {
            QString value;
            QString new_value;

            auto meta = editorObjectManager()->enumMeta(value);
            JZNodeFlagEditDialog dlg(this);
            dlg.init(meta);
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
        m_widget = edit;
    }
    else if (widget == "JZNodeParamEditWidget")
    {
        auto edit = editorEnvironment()->editorManager()->delegate(m_dataType)->createEdit();
        edit->connect(edit, &JZNodeParamEditWidget::sigValueChanged, this, &JZNodeParamValueWidget::sigValueChanged);
        m_widget = edit;
    }
    else
    {
        Q_ASSERT(0);
    }
    m_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    layout()->addWidget(m_widget);
    setFocusProxy(m_widget);

    blockSignals(false);
}

QWidget *JZNodeParamValueWidget::focusWidget()
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
    else if (m_widget->inherits("JZNodeParamPopupWidget"))
    {
        auto *edit = qobject_cast<JZNodeParamPopupWidget*>(m_widget);
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
        box->setText(flag ? "true" : "false");
    }
    else if (m_widget->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(m_widget);
        if (m_dataType == Type_int)
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
    else if (m_widget->inherits("JZNodeParamPopupWidget"))
    {
        auto *edit = qobject_cast<JZNodeParamPopupWidget*>(m_widget);
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

//JZNodeImageEditWidget
JZNodeImageEditWidget::JZNodeImageEditWidget()
{
    m_pixmapLabel = new QLabel(this);
    m_pathLabel = new QLabel(this);
    m_button = new QToolButton(this);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(m_pixmapLabel);
    layout->addWidget(m_pathLabel);
    layout->setMargin(0);
    layout->setSpacing(0);
    m_pixmapLabel->setFixedWidth(16);
    m_pixmapLabel->setAlignment(Qt::AlignCenter);
    m_pathLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));

    m_button->setText(tr("..."));
    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(30);
    layout->addWidget(m_button);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));
    connect(m_button, &QAbstractButton::clicked, this, &JZNodeImageEditWidget::onFileActionActivated);
}

JZNodeImageEditWidget::~JZNodeImageEditWidget()
{

}

QString JZNodeImageEditWidget::value() const
{
    return m_path;
}

void JZNodeImageEditWidget::setValue(const QString &text)
{
    m_path = text;

    if (m_path.isEmpty()) {
        m_pathLabel->setText(m_path);
        m_pixmapLabel->setPixmap(QPixmap());
    }
    else {
        QPixmap pixmap(m_path);
        pixmap = pixmap.scaled(16, 16, Qt::KeepAspectRatio);

        m_pathLabel->setText(QFileInfo(m_path).fileName());
        m_pixmapLabel->setPixmap(pixmap);
    }
}

void JZNodeImageEditWidget::onFileActionActivated()
{
    const QString newPath = QFileDialog::getOpenFileName(this, m_path, "", "*.bmp;*.jpg;*.png");
    if (!newPath.isEmpty() && newPath != m_path) {
        setValue(newPath);
        emit sigValueChanged();
    }
}