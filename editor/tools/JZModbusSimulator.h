#ifndef JZ_MODBUS_SIMULATOR_H_
#define JZ_MODBUS_SIMULATOR_H_

#include <QMdiArea>
#include <QMainWindow>
#include <QTreeWidget>
#include <QTableWidget>
#include <QPlainTextEdit>
#include "UiCommon.h"
#include "3rd/jzmodbus/JZModbusMaster.h"
#include "3rd/jzmodbus/JZModbusSlaver.h"

class SimulatorWidget;
class JZModbusSimulator : public QWidget
{
    Q_OBJECT

public:
    JZModbusSimulator(QWidget *parent = nullptr);
    virtual ~JZModbusSimulator();    
    
    void closeAll();

signals:


protected slots :
    void onActionNew();
    void onActionClear();
    void onActionShowAll();
    void onContextMenu(QPoint pt);

    void onSimulatorStart();
    void onSimulatorStop();
    void onSimulatorSetting();

    void onProtoStrategyClicked();
    void onProtoReadClicked();
    void onProtoWriteClicked();
    void onItemChanged(QTableWidgetItem *item);
    void onParamChanged(int addr);

    void onActionSaveConfig();
    void onActionLoadConfig();

protected:
    struct Simulator
    {
        Simulator();
        bool isOpen();
        void close();

        JZModbusMaster *master;
        JZModbusSlaver *slaver;
        
        QTreeWidgetItem *item;
        QTableWidget *table;
        QMdiSubWindow *window;        
        SimulatorWidget *widget;
        JZModbusConfig config;
    };
    
    virtual bool eventFilter(QObject *o, QEvent *e) override;
    void addSimulator(JZModbusConfig config);
    void removeSimulator(int index);
    void startSimulator(int index);
    void stopSimulator(int index);
    void settingSimulator(int index);
    void initSimulator(int index);    
    
    void updateStatus(int index);
    void updateTable(int index);
    int indexOfRow(QTableWidget *table,int addr);
    int indexOfTable(QTableWidget *table);
            
    QList<QSerialPort::DataBits> m_dataBitsList;
    QList<QSerialPort::StopBits> m_stopBitsList;
    QList<QSerialPort::Parity> m_parityList;          

    QList<Simulator> m_simulator;
    QMdiArea *m_mdiArea;
    QPlainTextEdit *m_log;
    QTreeWidget *m_tree;
    int m_simIdx;
};

#endif
