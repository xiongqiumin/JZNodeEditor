#include "JZNodeQtBind.h"
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include "JZNodeType.h"

bool JZBodeQtBind::isBindSupport(QWidget *w, const QVariant &v)
{
    return true;
}

bool JZBodeQtBind::uiToData(QWidget *w, QVariant &v)
{
    auto type = JZNodeType::variantType(v);
    if (w->inherits("QLineEdit"))
    {
        QLineEdit *edit = qobject_cast<QLineEdit*>(w);
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

bool JZBodeQtBind::dataToUi(const QVariant &v, QWidget *w)
{
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
    return true;
}

bool JZBodeQtBind::bind(QWidget *w)
{    
    auto change_func = [w] {
        extern void JZWidgetValueChanged(QWidget *w);
        JZWidgetValueChanged(w);
    };

    if (w->inherits("QLineEdit"))
    {
        QLineEdit *edit = qobject_cast<QLineEdit*>(w);
        edit->connect(edit, &QLineEdit::editingFinished, change_func);
    }
    else if (w->inherits("QComboBox"))
    {
        QComboBox *box = qobject_cast<QComboBox*>(w);
        box->connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged), change_func);
    }
    else if (w->inherits("QCheckBox"))
    {
        QCheckBox *box = qobject_cast<QCheckBox*>(w);
        box->connect(box, &QCheckBox::clicked, change_func);
    }
    return true;
}