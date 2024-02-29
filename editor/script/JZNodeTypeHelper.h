#ifndef JZNODE_NEW_TYPE_HELPER_H_
#define JZNODE_NEW_TYPE_HELPER_H_

#include <QDialog>
#include <QStyledItemDelegate>
#include "UiCommon.h"
#include "JZNodeType.h"

namespace Ui { class JZNodeTypeDialog; }

class QComboBox;
class TypeEditHelp
{
public:
    TypeEditHelp();
    void init(QString dataType);
    void update(QComboBox *box);

    int index;
    QVector<int> types;
    QStringList typeNames;
};

class TypeItemDelegate : public QStyledItemDelegate
{
public:
    TypeItemDelegate(QObject *parent);

protected:
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

//JZNodeTypeDialog
class JZNodeTypeDialog : public QDialog
{
    Q_OBJECT
    
public:
    JZNodeTypeDialog(QWidget *p = nullptr);
    ~JZNodeTypeDialog();
    
    void setDataType(QString dataType);
    QString dataType();

protected slots:
    void on_lineClassName_returnPressed();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

protected:
    QString m_dataType;
    Ui::JZNodeTypeDialog *ui;
};

#endif
