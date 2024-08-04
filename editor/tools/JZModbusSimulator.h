#ifndef JZ_MODBUS_SIMULATOR_H_
#define JZ_MODBUS_SIMULATOR_H_

#include <QMainWindow>
#include "ui_JZModbusSimulator.h"
#include "UiCommon.h"
#include "3rd/jzmodbus/JZModbusMaster.h"
#include "3rd/jzmodbus/JZModbusSlaver.h"

enum {
    Modbus_rtuClient,
    Modbus_tcpClient,
    Modbus_rtuServer,
    Modbus_tcpServer,
};

class JZModbusSimulator : public QDialog
{
    Q_OBJECT

public:
    JZModbusSimulator(QWidget *parent = nullptr);
    virtual ~JZModbusSimulator();

    void run();
    void setConfigMode(bool isRtu);
    void setModbusType(int type);

    void setConfig(const JZModbusConfig &cfg);
    JZModbusConfig config();    
    
signals:

protected slots :
    void onBoxTypeChanged();
    void on_btnStart_clicked();
    void on_btnSetting_clicked();
    void on_btnClose_clicked();

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
    int currentModbusType();
    
    JZModbusMaster *m_master;
    JZModbusSlaver *m_slaver;    
    bool m_run;    
    
    QMap<int, JZModbusStrategy> m_strategyMap;

    QList<int> m_baudRateList;
    QList<QSerialPort::DataBits> m_dataBitsList;
    QList<QSerialPort::StopBits> m_stopBitsList;
    QList<QSerialPort::Parity> m_parityList;    

    int m_rtuSlave;
    int m_tcpSlave;

    QString m_portName;
    int m_baud,m_dataBit,m_parityBit,m_stopBit;

    QString m_ip;
    int m_port;

    Ui::JZModbusSimulatorClass ui;
};


class JZModbusSimulatorManager : public QObject
{
    Q_OBJECT

public:
    static JZModbusSimulatorManager *instance();

    void addSimulator(JZModbusSimulator *simulator);
    void closeAll();

protected slots:
    void onSimulatorClose();

protected:
    QList<JZModbusSimulator*> m_modbusList;
};





#endif
