#include "JZNodeQtBind.h"
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include "JZNodeType.h"

JZNodeQtBindHelper::JZNodeQtBindHelper(QWidget *parent)
    :QObject(parent)
{
    m_blockChanged = false;
}

JZNodeQtBindHelper::~JZNodeQtBindHelper()
{
    extern void JZWidgetUnBindNotify(QWidget *w);
    auto w = qobject_cast<QWidget*>(parent());
    JZNodeQtBind::unbind(w);
    JZWidgetUnBindNotify(w);
}

void JZNodeQtBindHelper::blockChanged(bool flag)
{
    m_blockChanged = flag;
}

void JZNodeQtBindHelper::valueChanged()
{
    extern void JZWidgetValueChanged(QWidget *w);
    if (m_blockChanged)
        return;

    auto w = qobject_cast<QWidget*>(parent());
    JZWidgetValueChanged(w);
}

//JZNodeQtBind
bool JZNodeQtBind::isBindSupport(QWidget *w, int type)
{
    return true;
}

bool JZNodeQtBind::uiToData(QWidget *w, QVariant &v)
{
    auto type = JZNodeType::variantType(v);    
    if (w->inherits("QLineEdit"))
    {
        QLineEdit *edit = qobject_cast<QLineEdit*>(w);
        if (type == Type_int)
            v = edit->text().toInt();
        else if (type == Type_double)
            v = edit->text().toDouble();
        else if (type == Type_string)
            v = edit->text();
    }
    else if (w->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(w);
        if (type == Type_int)
            v = box->currentIndex();
        else if (type == Type_string)
            v = box->currentText();
    }
    else if (w->inherits("QCheckBox"))
    {
        QCheckBox *box = qobject_cast<QCheckBox*>(w);
        if (type == Type_int)
            v = (int)box->isChecked();
        else if(type == Type_bool)
            v = box->isChecked();
    }
    return true;
}

bool JZNodeQtBind::dataToUi(const QVariant &v, QWidget *w)
{
    auto helper = w->findChild<JZNodeQtBindHelper*>();
    helper->blockChanged(true);

    auto type = JZNodeType::variantType(v);
    if (w->inherits("QLineEdit"))
    {
        QLineEdit *edit = qobject_cast<QLineEdit*>(w);
        edit->setText(v.toString());
    }
    else if (w->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(w);
        if(type == Type_int)
            box->setCurrentIndex(v.toInt());
        else if (type == Type_string)
            box->setCurrentText(v.toString());
    }
    else if (w->inherits("QCheckBox"))
    {
        QCheckBox *box = qobject_cast<QCheckBox*>(w);
        if (type == Type_int || type == Type_bool)
            box->setChecked(v.toBool());
    }
    helper->blockChanged(false);
    return true;
}

bool JZNodeQtBind::bind(QWidget *w,QVariant *v)
{    
    JZNodeQtBindHelper *helper = new JZNodeQtBindHelper(w);
    
    w->setProperty("BindValue", QVariant::fromValue((void*)v));
    if (w->inherits("QLineEdit"))
    {
        QLineEdit *edit = qobject_cast<QLineEdit*>(w);
        edit->connect(edit, &QLineEdit::editingFinished, helper, &JZNodeQtBindHelper::valueChanged);
    }
    else if (w->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(w);
        box->connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged), helper, &JZNodeQtBindHelper::valueChanged);
    }
    else if (w->inherits("QCheckBox"))
    {
        QCheckBox *box = qobject_cast<QCheckBox*>(w);
        box->connect(box, &QCheckBox::clicked, helper, &JZNodeQtBindHelper::valueChanged);
    }
    return true;
}

void JZNodeQtBind::unbind(QWidget *w)
{    
    w->setProperty("BindValue",QVariant());    
}