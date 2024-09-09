#ifndef JZ_MODBUS_CONFIG_DIALOG_H_
#define JZ_MODBUS_CONFIG_DIALOG_H_

#include <QMainWindow>
#include <QStackedWidget>
#include "ui_JZModbusConfigDialog.h"
#include "UiCommon.h"
#include "3rd/jzmodbus/JZModbusMaster.h"
#include "3rd/jzmodbus/JZModbusSlaver.h"
#include "JZBaseDialog.h"


//ModbusStargeDialog
class ModeStargeDialog : public JZBaseDialog
{
    Q_OBJECT

public:
    ModeStargeDialog(QWidget *widget = nullptr);

    void setInfo(JZModbusStrategy info);
    JZModbusStrategy info();
    
protected:
    virtual bool onOk() override;

    QCheckBox *m_boxRead;
    QCheckBox *m_boxRecv;
    QLineEdit *m_lineTime;
};

class JZModbusConfigDialog : public QDialog
{
    Q_OBJECT

public:
    JZModbusConfigDialog(QWidget *parent = nullptr);
    virtual ~JZModbusConfigDialog();

    void setConfigMode(bool isRtu);

    void setConfig(const JZModbusConfig &cfg);
    JZModbusConfig config();    
    
signals:

protected slots :
    void onBoxTypeChanged();    

    void on_btnSelectTemplate_clicked();
    void on_btnClose_clicked();

    void on_btnAdd_clicked();
    void on_btnEdit_clicked();
    void on_btnRemove_clicked();

    void onProtoStrategyClicked();
    
protected:        
    void initComm();
    void updateTable();
    int indexOfRow(int addr);
    void setCurrentModbusType(int type);
    int currentModbusType();           

    int m_rtuSlave;
    int m_tcpSlave;    

    JZModbusParamMap m_map;
    QMap<int, JZModbusStrategy> m_strategyMap;

    QStackedWidget *m_stack;
    QComboBox *m_comboBoxCom, *m_comboBoxBotelv, *m_comboBoxData, *m_comboBoxStop, *m_comboBoxChk;
    QLineEdit *m_lineIp, *m_linePort, *m_linePortServer;

    Ui::JZModbusConfigDialogClass ui;
};


#endif
