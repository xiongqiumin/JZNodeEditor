#ifndef JZ_CAMERAL_MANAGER_EDITOR_H_
#define JZ_CAMERAL_MANAGER_EDITOR_H_

#include "JZNode.h"
#include "JZNodeGraphItem.h"
#include "JZNodeSettingDialog.h"
#include "JZCameraManager.h"
#include "JZBaseDialog.h"
#include "jzWidgets/JZPropertyBrowser.h"
#include "JZCameraWidget.h"

//JZCameraInitDialog
class JZCameraInitDialog : public JZNodeManagerDialog
{
public:    
    JZCameraInitDialog(QWidget *parent);

    void setConfig(JZCameraManagerConfig cfg);
    JZCameraManagerConfig config();    

    virtual void addConfig() override;
    virtual void removeConfig(int index) override;
    virtual void settingConfig(int index) override;
    virtual void updateConfig() override;

protected:
    JZCameraManagerConfig m_config;
    QStringList m_camTypeList;
};

class JZCameraInitItem : public JZNodeGraphItem
{
public:
    JZCameraInitItem(JZNode *node);

    virtual void updatePin();

protected:    
    void onSetClicked();
    
    BlockPtr m_setting;
};

class JZCameraNodeItem: public JZNodeGraphItem
{
public:
    JZCameraNodeItem(JZNode *node);

    virtual void updatePin();

protected:
};

void JZCameraEditorInit();


#endif // !JZ_CAMERAL_MANAGER_EDITOR_H_
