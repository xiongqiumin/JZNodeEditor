#pragma once

#include <QDialog>
#include <QTreeWidget>
#include "JZBaseDialog.h"
#include "JZNode.h"

class JZVisionLinkDialog : public JZBaseDialog
{
    Q_OBJECT
    
public:
    explicit JZVisionLinkDialog(QWidget *parent = nullptr);
    ~JZVisionLinkDialog();
    
    void setNode(JZNode* node, int pin_id);
    JZNodeGemo result();
protected:
    void addLinkItem(JZNode* node,const QList<int> &dst_types);
    virtual void accept() override;

    QTreeWidget *m_tree;
    JZNodeGemo m_result;
};