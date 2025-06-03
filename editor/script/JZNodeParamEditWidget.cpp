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
#include <QTimer>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "JZEditorGlobal.h"
#include "JZNodeParamEditWidget.h"
#include "JZNodeTypeHelper.h"
#include "JZNodeFlagEditDialog.h"
#include "JZNodeEditorManager.h"

//JZNodeParamTypeWidget
JZNodeParamTypeWidget::JZNodeParamTypeWidget()
{
    m_lineEdit = new QLineEdit();

    QHBoxLayout *h = new QHBoxLayout();
    h->setContentsMargins(0, 0, 0, 0);
    h->addWidget(m_lineEdit);
    setLayout(h);
}

void JZNodeParamTypeWidget::setType(QString type)
{
    m_lineEdit->setText(type);
}

QString JZNodeParamTypeWidget::type()
{
    return m_lineEdit->text();
}

//JZParamEditInfo
JZParamEditInfo JZParamEditInfo::createEnum(QStringList list)
{
    JZParamEditInfo info;
    info.type = Edit_enum;
    info.enumList = list;
    return info;
}

JZParamEditInfo JZParamEditInfo::createType(const JZScriptEnvironment* env, QString type)
{
    JZParamEditInfo info;

    int type_id = env->nameToType(type);
    if (type_id == Type_none)
        return info;

    if (type_id == Type_bool)
    {
        info.type = Edit_bool;
    }
    else if (type_id == Type_int8 || type_id == Type_int16 || type_id == Type_int
        || type_id == Type_uint8 || type_id == Type_uint16)
    {
        info.type = Edit_int;
        if (type_id == Type_int8)
        {
            info.min = INT8_MIN;
            info.max = INT8_MAX;
        }
        else if (type_id == Type_uint8)
        {
            info.min = 0;
            info.max = UINT8_MAX;
        }
        else if (type_id == Type_int16)
        {
            info.min = INT16_MIN;
            info.max = INT16_MAX;
        }
        else if (type_id == Type_uint16)
        {
            info.min = 0;
            info.max = UINT16_MAX;
        }
        else if (type_id == Type_int)
        {
            info.min = INT_MIN;
            info.max = INT_MAX;
        }
    }
    else if (type_id == Type_uint || type_id == Type_int64 || type_id == Type_uint64)
    {
        info.type = Edit_normal;
    }
    else if (type_id == Type_float || type_id == Type_double)
    {
        info.type = Edit_double;
    }
    else if (type_id == Type_string)
    {
        info.type = Edit_normal;
    }
    else if (type_id == Type_byteArray)
    {
        info.type = Edit_byteArray;
    }
    else if (env->isEnum(type))
    {
        auto meta = env->objectManager()->enumMeta(type);

        info.type = Edit_enum;
        info.enumList = meta->keys();
    }

    
    return info;
}

JZParamEditInfo::JZParamEditInfo()
{
    type = Edit_none;
}

//JZNodeParamValueWidget
JZNodeParamValueWidget::JZNodeParamValueWidget()
{
    QHBoxLayout *h = new QHBoxLayout();
    h->setContentsMargins(0, 0, 0, 0);
    setLayout(h);

    m_editWidget = nullptr; 
}

void JZNodeParamValueWidget::init(const JZParamEditInfo &edit)
{
    if(m_editWidget)
        delete m_editWidget;

    if(edit.type == JZParamEditInfo::Edit_normal)
    {
        QLineEdit *lineEdit = new QLineEdit();
        connect(lineEdit, &QLineEdit::returnPressed, this, &JZNodeParamValueWidget::sigEditFinish); 
        m_editWidget = lineEdit;
    }
    else if(edit.type == JZParamEditInfo::Edit_enum)
    {
        QComboBox *box = new QComboBox();
        box->addItems(edit.enumList);

        connect(box, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigEditFinish()));          
        m_editWidget = box;
    }
    else
    {
        Q_ASSERT(0);
    }

    layout()->addWidget(m_editWidget);
    setFocusProxy(m_editWidget);
}

void JZNodeParamValueWidget::setValue(QString value)
{
    if(m_editWidget->inherits("QLineEdit"))
    {
        auto line_edit = qobject_cast<QLineEdit*>(m_editWidget);
        line_edit->setText(value);
    }
    else if (m_editWidget->inherits("QSpinBox"))
    {
        auto spin = qobject_cast<QSpinBox*>(m_editWidget);
        spin->setValue(value.toInt());
    }
    else if (m_editWidget->inherits("QDoubleSpinBox"))
    {
        auto spin = qobject_cast<QDoubleSpinBox*>(m_editWidget);
        spin->setValue(value.toDouble());
    }
    else if(m_editWidget->inherits("QComboBox"))
    {
        auto box = qobject_cast<QComboBox*>(m_editWidget);
        box->setCurrentText(value);
    }
    else
    {
        Q_ASSERT(0);
    }
}

QString JZNodeParamValueWidget::value()
{
    if(m_editWidget->inherits("QLineEdit"))
    {
        auto line_edit = qobject_cast<QLineEdit*>(m_editWidget);
        return line_edit->text();
    }
    else if (m_editWidget->inherits("QSpinBox"))
    {
        auto spin = qobject_cast<QSpinBox*>(m_editWidget);
        return QString::number(spin->value());
    }
    else if (m_editWidget->inherits("QDoubleSpinBox"))
    {
        auto spin = qobject_cast<QDoubleSpinBox*>(m_editWidget);
        return QString::number(spin->value());
    }
    else if(m_editWidget->inherits("QComboBox"))
    {
        auto box = qobject_cast<QComboBox*>(m_editWidget);
        return box->currentText();
    }
    else
    {
        Q_ASSERT(0);
        return QString();
    }
}