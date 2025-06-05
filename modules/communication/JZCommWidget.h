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
    ~JZCommConfigDialog();

    void setConfig(JZCommConfigEnum cfg);
    JZCommConfigEnum getConfig() const;

private slots:
    

private:    
    void addModbus();
    void addTcpClient();
    void addTcpServer();
    void addUdp();
    void addCom();

    void accept();    

    QList<int> bit_order_value;
    QStringList bit_order_text;
    QList<int> baud_value;
    QStringList baud_text;
    QList<int> dataBit_value;
    QStringList dataBit_text;
    QList<int> parityBit_value;
    QStringList parityBit_text;
    QList<int> stopBit_value;
    QStringList stopBit_text;

    int m_type;
    QString m_name;

    JZProperty *m_propGroup;
    JZProperty *m_commGroup;
    QMap<int, JZCommConfigEnum*> m_config;
};


//JZCommConfigWidget
class JZCommConfigWidget : public JZModuleConfigWidget
{
    Q_OBJECT

public:    
    JZCommConfigWidget(QWidget *parent = nullptr);

    void setConfig(JZCommManagerConfig cfg);
    JZCommManagerConfig config();    

    virtual void addConfig() override;
    virtual void removeConfig(int index) override;
    virtual void settingConfig(int index) override;
    virtual void updateConfig() override;

signals:
    void sigCommChanged();

protected:
    JZCommManagerConfig m_config;
    QStringList m_camTypeList;
};





#endif