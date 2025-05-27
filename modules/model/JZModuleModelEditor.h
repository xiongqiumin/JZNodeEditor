#ifndef JZ_MODEL_MANAGER_EDITOR_H_
#define JZ_MODEL_MANAGER_EDITOR_H_

#include "JZModelManager.h"
#include "JZNodeSettingDialog.h"
#include "JZNodeGraphItem.h"
#include "JZPropertyDialog.h"
#include "JZModelWidget.h"

//JZModelInitDialog
class JZModelInitDialog : public JZNodeManagerDialog
{
public:
    JZModelInitDialog(QWidget *parent);

    void setConfig(JZModelManagerConfig cfg);
    JZModelManagerConfig config();

    virtual void addConfig() override;
    virtual void removeConfig(int index) override;
    virtual void settingConfig(int index) override;
    virtual void updateConfig() override;

protected:
    JZModelManagerConfig m_config;
    QStringList m_modelTypeList;
};

class JZModelInitItem : public JZNodeGraphItem
{
public:
    JZModelInitItem(JZNode *node);

    virtual void updatePin();

protected:
    void onSetClicked();

    BlockPtr m_setting;
};


class JZModelForwardItem : public JZNodeGraphItem
{
public:
    JZModelForwardItem(JZNode *node);
    
protected:
};

void JZModuleModelEditorInit();

#endif // !JZ_CAMERAL_MANAGER_EDITOR_H_
