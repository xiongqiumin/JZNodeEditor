#pragma once

#include <QDialog>
#include <QTableWidget>
#include "JZNodeFunction.h"
#include "JZScriptItem.h"
#include "JZBaseDialog.h"

class QCheckBox;
class JZNodeFlagEditDialog : public JZBaseDialog
{
    
public:
    JZNodeFlagEditDialog(QWidget *parent = Q_NULLPTR);
    ~JZNodeFlagEditDialog();
    
    void init(QString flagName);
    void setFlag(QString flag);
    QString flag();    
    
private:    
    virtual bool onOk() override;

    QString m_flag;    
    QString m_flagKey;
    QList<QCheckBox*> m_boxList;
};


class JZNodeNumberStringEditDialog : public JZBaseDialog
{
public:
    JZNodeNumberStringEditDialog(QWidget *parent = Q_NULLPTR);
    ~JZNodeNumberStringEditDialog();
    
    void setValue(QString flag);
    QString value();

private:
    virtual bool onOk() override;

    QComboBox *m_box;
    QLineEdit *m_line;
};