#ifndef JZ_MODBUS_SIMULATOR_H_
#define JZ_MODBUS_SIMULATOR_H_

#include <QMainWindow>
#include "ui_JZModbusSimulator.h"
#include "UiCommon.h"
#include "3rd/jzmodbus/JZModbusMaster.h"
#include "3rd/jzmodbus/JZModbusSlaver.h"

class JZModbusSimulator : public QMainWindow
{
    Q_OBJECT

public:
    JZModbusSimulator(QWidget *parent = nullptr);
    virtual ~JZModbusSimulator();
    
signals:

protected slots :
    void onBoxTypeChanged();
    void on_btnStart_clicked();
    void on_btnSetting_clicked();

    void on_btnAdd_clicked();
    void on_btnEdit_clicked();
    void on_btnRemove_clicked();       

    void onProtoReadClicked();
    void onProtoWriteClicked();
    void onProtoStrategyClicked();

    void onItemChanged(QTableWidgetItem *item);
    void onParamChanged(int addr);

protected:
    void init();
    JZModbusParamMap *mapping();
    void updateStatus();
    void updateTable();
    int indexOfRow(int addr);
    
    JZModbusMaster *m_master;
    JZModbusSlaver *m_slaver;    
    bool m_run;    
    
    QMap<int, JZModbusStrategy> m_strategy;

    QList<int> m_baudRateList;
    QList<QSerialPort::DataBits> m_dataBitsList;
    QList<QSerialPort::StopBits> m_stopBitsList;
    QList<QSerialPort::Parity> m_parityList;    
    QString m_portName;
    int m_baud,m_dataBit,m_parityBit,m_stopBit;

    QString m_ip;
    int m_port, m_serverPort;

    Ui::JZModbusSimulatorClass ui;
};








#endif
