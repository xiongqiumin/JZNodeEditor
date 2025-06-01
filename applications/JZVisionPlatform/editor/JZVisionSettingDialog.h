#pragma once

#include <QDialog>
#include "jzWidgets/JZPropertyBrowser.h"
#include "JZBaseDialog.h"
#include "JZNode.h"

class JZVisionSettingDialog : public JZBaseDialog
{
    Q_OBJECT
    
public:
    struct Block
    {
        bool isLink();

        int pinId;
        JZNodeGemo linkGemo;
        QString value;
        QLineEdit *line;
        QString error;
    };

    explicit JZVisionSettingDialog(QWidget *parent = nullptr);
    ~JZVisionSettingDialog();

    void setNode(JZNode* node);
    QMap<int, Block> blockList();

protected slots:
    void onBtnLink();

protected:    
    QWidget *createRow(QString name, QString value);
    QWidget *createPin(JZNodePin *pin);
    void updateBlock(int id);

    void accept();

    QMap<int,Block> m_blockList;
    JZNode* m_node;
};