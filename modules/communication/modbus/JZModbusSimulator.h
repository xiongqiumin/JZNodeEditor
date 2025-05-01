#ifndef JZ_MODBUS_SIMULATOR_H_
#define JZ_MODBUS_SIMULATOR_H_

#include <QMdiArea>
#include <QMainWindow>
#include <QTreeWidget>
#include <QTableWidget>
#include <QPlainTextEdit>
#include "UiCommon.h"
#include "3rd/JZCommon/jzModbus/JZModbusMaster.h"
#include "3rd/JZCommon/jzModbus/JZModbusSlaver.h"

class SimulatorWidget;

class JZModbusSimulatorConfig
{
public:
    QList<JZModbusConfig> modbusList;
};
QDataStream &operator<<(QDataStream &s, const JZModbusSimulatorConfig &param);
QDataStream &operator>>(QDataStream &s, JZModbusSimulatorConfig &param);

//这里使用 mainwindow ， 因为mdiarea 放大窗口会和菜单重合
class JZModbusSimulator : public QWidget
{
    Q_OBJECT

public:
    JZModbusSimulator(QWidget *parent = nullptr);
    virtual ~JZModbusSimulator();    
    
    void setConfig(JZModbusSimulatorConfig config);
    JZModbusSimulatorConfig config();
    void closeAll();

signals:
    void sigClose();

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

    void onItemDoubleClicked(QTreeWidgetItem *item);

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
    
    virtual void closeEvent(QCloseEvent *event) override;
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
