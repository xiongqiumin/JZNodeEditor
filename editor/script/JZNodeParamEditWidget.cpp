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

//JZNodeParamValueWidget
JZNodeParamValueWidget::JZNodeParamValueWidget()
{
    m_lineEdit = new QLineEdit();
    setFocusProxy(m_lineEdit);
    connect(m_lineEdit, &QLineEdit::returnPressed, this, &JZNodeParamValueWidget::sigEditFinish);

    QHBoxLayout *h = new QHBoxLayout();
    h->setContentsMargins(0, 0, 0, 0);
    h->addWidget(m_lineEdit);
    setLayout(h);

    m_lineEdit->installEventFilter(this);
}

void JZNodeParamValueWidget::initWidget(int type)
{

}

void JZNodeParamValueWidget::setValue(QString type)
{
    m_lineEdit->setText(type);
}

QString JZNodeParamValueWidget::value()
{
    return m_lineEdit->text();
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