#ifndef JZ_MODBUS_SIMULATOR_H_
#define JZ_MODBUS_SIMULATOR_H_

#include <QTableWidget>
#include <QToolButton>
#include "JZModbusConfigDialog.h"
#include "../JZCommSimulatorWidget.h"

class JZModBusSimulator : public JZCommSimulatorWidget
{
    Q_OBJECT

public:
    JZModBusSimulator();
    ~JZModBusSimulator();

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;
    virtual void setting() override;
    virtual void setConfig(const QByteArray &buffer) override;
    virtual QByteArray getConfig() override;
    
protected slots:
    void onProtoStrategyClicked();
    void onProtoReadClicked();
    void onProtoWriteClicked();
    void onItemChanged(QTableWidgetItem *item);
    void onParamChanged(int addr);

    void onSimulatorStart();
    void onSimulatorStop();
    void onSimulatorSetting();    

protected:
    void initSimulator();
    void updateStatus();
    void updateTable();

    int indexOfRow(int addr);    
    void startSimulator();
    void stopSimulator();
    void settingSimulator();

    QToolButton *m_btnStart, *m_btnStop, *m_btnSetting;
    QTableWidget *m_table;
    JZModbusMaster *m_master;
    JZModbusSlaver *m_slaver;
    JZModbusConfig m_config;
};








#endif