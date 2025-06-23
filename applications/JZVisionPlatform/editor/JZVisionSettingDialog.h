#pragma once

#include <QDialog>
#include <QTreeWidget>
#include "jzWidgets/JZPropertyBrowser.h"
#include "JZBaseDialog.h"
#include "JZNode.h"
#include "JZNodeParamEditWidget.h"
#include "JZLineEditButton.h"
#include "JZVisionAppNode.h"

class JZVisionView;

//JZVisionLinkDialog
class JZVisionLinkDialog : public JZBaseDialog
{
public:
    JZVisionLinkDialog(QWidget *w);

    void initLinkList(JZNode *node,int pin_id);
    void setLink(JZVisionParamLink link);
    JZVisionParamLink link();

protected:
    virtual void accept() override;
    void addLinkItem(QTreeWidgetItem *parent,const JZVisionParamLink &link, const JZParamDefine* param,const QList<int> &dst_types);

    QTreeWidget *m_tree;
    QMap<int,JZVisionParamLink> m_paramLink;
    JZNode *m_node;
    int m_pinId;
    int m_linkId;
};

//JZVisionSettingPinWidget
class JZVisionSettingDialog;
class JZVisionSettingPinWidget : public QWidget
{
    Q_OBJECT

public:
    JZVisionSettingPinWidget();
    ~JZVisionSettingPinWidget();

    void setPin(JZNode* node, int pin_id);
    
    JZVisionSettingDialog *setting();
    void setSetting(JZVisionSettingDialog *dlg);

    bool isLink();
    JZVisionParamLink linkInfo();
    QString value();

protected slots:
    void onBtnLink();
    void onLickSelected();

protected:
    QString linkName();
    void updatePinWidget();

    JZNode* m_node;
    int m_pinId;

    bool m_isLink;
    QToolButton* m_btnLink;
    JZVisionParamLink m_linkGemo;

    JZNodeParamValueWidget* m_pinEditor;
    JZLineEditButton* m_linkEdit;

    JZVisionSettingDialog *m_setting;
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

public slots:
    void onPinRemove();
    void onPinAdd();
    void onPinElse();

protected:    
    QWidget *createRow(QString name, QString value);
    JZVisionSettingPinWidget* createPin(JZNodePin *pin);

    void accept();

    QMap<int,Block> m_blockList;
    JZNode* m_node;
    QVBoxLayout* m_grid;
};