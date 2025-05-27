#ifndef JZ_COMM_WIDGET_H_
#define JZ_COMM_WIDGET_H_

#include "JZPropertyDialog.h"
#include "JZCommManager.h"
#include "../JZModuleConfigWidget.h"

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


//JZCommConfigWidget
class JZCommConfigWidget : public JZModuleConfigWidget
{
public:    
    JZCommConfigWidget(QWidget *parent = nullptr);

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





#endif