#ifndef JZ_MODULE_COMM_EDITOR_H_
#define JZ_MODULE_COMM_EDITOR_H_

#include "JZCommManager.h"
#include "JZNodeSettingDialog.h"
#include "JZNodeGraphItem.h"

//JZCommConfigDialog
class JZCommConfigDialog : public JZManagerPropertyDialog
{
    Q_OBJECT

public:
    JZCommConfigDialog(QWidget *parent = nullptr);

    void setConfig(JZCommConfig cfg);
    JZCommConfig getConfig() const;

private slots:
    

private:    
    void accept();

    JZCommConfig m_config;

};

//JZCommInitDialog
class JZCommInitDialog : public JZNodeManagerDialog
{
public:    
    JZCommInitDialog(QWidget *parent);

    void setConfig(JZCommManagerConfig cfg);
    JZCommManagerConfig config();    

    virtual void addConfig() override;
    virtual void removeConfig(int index) override;
    virtual void settingConfig(int index) override;
    virtual void updateConfig() override;

protected:
    JZCommManagerConfig m_config;
    QStringList m_camTypeList;
};

class JZCommInitItem : public JZNodeGraphItem
{
public:
    virtual void updatePin();

protected:    
    void onSetClicked();
    
    BlockPtr m_setting;
};

void JZModuleCommEditorInit();


#endif // !JZ_CAMERAL_MANAGER_EDITOR_H_
