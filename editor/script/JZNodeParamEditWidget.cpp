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

//JZParamEdit
JZParamEdit::JZParamEdit()
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

void JZNodeParamValueWidget::init(const JZParamEdit &edit)
{
    if(m_editWidget)
        delete m_editWidget;

    if(edit.type == JZParamEdit::Edit_normal)
    {
        QLineEdit *lineEdit = new QLineEdit();
        connect(lineEdit, &QLineEdit::returnPressed, this, &JZNodeParamValueWidget::sigEditFinish); 
        m_editWidget = lineEdit;
    }
    else if(edit.type == JZParamEdit::Edit_enum)
    {
        QComboBox *box = new QComboBox();
        box->addItems(edit.enumList);

        connect(box, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigEditFinish()));  
        QTimer::singleShot(0, [box]() {        
            box->showPopup();        
        });
        m_editWidget = box;
    }
    else
    {
        Q_ASSERT(0);
    }

    layout()->addWidget(m_editWidget);
    setFocusProxy(m_editWidget);
    m_editWidget->installEventFilter(this);
}

void JZNodeParamValueWidget::setValue(QString value)
{
    if(m_editWidget->inherits("QLineEdit"))
    {
        auto line_edit = qobject_cast<QLineEdit*>(m_editWidget);
        line_edit->setText(value);
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

bool JZNodeParamValueWidget::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type())
    {    
    case QEvent::FocusOut:   
    {
        emit sigEditFinish();
        break;
    }
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}