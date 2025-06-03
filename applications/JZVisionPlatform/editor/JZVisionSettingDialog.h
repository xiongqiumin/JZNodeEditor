#pragma once

#include <QDialog>
#include "jzWidgets/JZPropertyBrowser.h"
#include "JZBaseDialog.h"
#include "JZNode.h"
#include "JZNodeParamEditWidget.h"

class JZVisionView;

//JZVisionSettingPinWidget
class JZVisionSettingPinWidget : public QWidget
{
    Q_OBJECT

public:
    JZVisionSettingPinWidget();

    void setPin(JZNode* node, int pin_id);

    bool isLink();
    JZNodeGemo linkGemo();
    QString value();

protected slots:
    void onBtnLink();

protected:
    void updatePinWidget();

    JZNode* m_node;
    int m_pinId;

    JZNodeParamValueWidget* m_pinEditor;
    QToolButton* m_btnLink;
    JZNodeGemo m_linkGemo;
    QLineEdit* m_linkTip;
};


//JZVisionSettingDialog
class JZVisionSettingDialog : public JZBaseDialog
{
    Q_OBJECT
    
public:
    struct Block
    {        
        int pinId;               
        JZVisionSettingPinWidget* pinWidget;
        QString error;
    };

    explicit JZVisionSettingDialog(QWidget *parent = nullptr);
    ~JZVisionSettingDialog();

    void setNode(JZNode* node);
    QMap<int, Block> blockList();
    JZVisionView* view();

protected slots:

protected:    
    QWidget *createRow(QString name, QString value);
    JZVisionSettingPinWidget*createPin(JZNodePin *pin);

    void accept();

    QMap<int,Block> m_blockList;
    JZNode* m_node;
};