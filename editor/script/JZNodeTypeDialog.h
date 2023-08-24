#ifndef JZNODE_NEW_TYPE_DIALOG_H_
#define JZNODE_NEW_TYPE_DIALOG_H_

#include <QDialog>
#include "UiCommon.h"
#include "JZNodeType.h"

namespace Ui { class JZNodeTypeDialog; }

class QComboBox;
class TypeEditHelp
{
public:
    TypeEditHelp();
    void init(int dataType);
    void update(QComboBox *box);

    int index;
    QVector<int> types;
    QStringList typeNames;
};

//JZNodeTypeDialog
class JZNodeTypeDialog : public QDialog
{
    Q_OBJECT
    
public:
    JZNodeTypeDialog(QWidget *p = nullptr);
    ~JZNodeTypeDialog();
    
    void setDataType(int dataType);
    int dataType();

protected slots:
    void on_lineClassName_returnPressed();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

protected:
    int m_dataType;    
    Ui::JZNodeTypeDialog *ui;
};

#endif
