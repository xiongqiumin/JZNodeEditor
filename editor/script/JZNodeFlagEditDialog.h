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
    
    void init(const JZNodeEnumDefine *meta);
    void setFlag(QString flag);
    QString flag();    
    
private:    
    virtual bool onOk() override;
    
    QString m_flagKey;
    const JZNodeEnumDefine *m_enumMeta;
    QList<QCheckBox*> m_boxList;
};