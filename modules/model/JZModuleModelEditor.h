#ifndef JZ_MODEL_MANAGER_EDITOR_H_
#define JZ_MODEL_MANAGER_EDITOR_H_

#include "JZModelManager.h"
#include "JZNodeSettingDialog.h"
#include "JZNodeGraphItem.h"

//JZModelConfigDialog
class JZModelConfigDialog : public JZManagerPropertyDialog
{
    Q_OBJECT

public:
    JZModelConfigDialog(QWidget *parent = nullptr);

    void setConfig(JZModelConfig cfg);
    JZModelConfig getConfig() const;

private slots:


private:
    void accept();

    JZModelConfig m_config;

};

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
    QStringList m_camTypeList;
};

class JZModelInitItem : public JZNodeGraphItem
{
public:
    virtual void updatePin();

protected:
    void onSetClicked();

    BlockPtr m_setting;
};

void JZModuleModelEditorInit();

#endif // !JZ_CAMERAL_MANAGER_EDITOR_H_
