#ifndef JZ_MODULE_COMM_EDITOR_H_
#define JZ_MODULE_COMM_EDITOR_H_

#include "JZCommManager.h"
#include "JZNodeSettingDialog.h"
#include "JZNodeGraphItem.h"
#include "JZPropertyDialog.h"

//JZCommConfigDialog
class JZCommConfigDialog : public JZPropertyDialog
{
    Q_OBJECT

public:
    JZCommConfigDialog(QWidget *parent = nullptr);

    void setConfig(JZCommConfigPtr cfg);
    JZCommConfigPtr getConfig() const;

private slots:
    

private:    
    void addModbusClient();
    void addModbusServer();
    void addTcpClient();
    void addTcpServer();
    void addUdp();
    void addCom();

    void accept();    

    int m_type;
    QString m_name;

    JZProperty *m_propGroup;
    JZProperty *m_commGroup;
    QMap<int, JZCommConfigPtr> m_config;
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
    JZCommInitItem(JZNode *node);

protected:    
    void onSetClicked();
    
    BlockPtr m_setting;
};


class JZCommModbusRWItem : public JZNodeGraphItem
{
public:
    JZCommModbusRWItem(JZNode *node);
    
    virtual void setBlockValue(int pin, QString value) override;
    virtual QString blockValue(int pin) override;

protected:
    virtual void updatePin() override;

    QStringList m_funcList;
    QStringList m_dataTypeList;

    BlockPtr m_modbusFunc;
    BlockPtr m_modbusDataType;
};

void JZModuleCommEditorInit();


#endif // !JZ_CAMERAL_MANAGER_EDITOR_H_
