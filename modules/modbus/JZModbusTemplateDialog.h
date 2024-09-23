#ifndef JZ_MODBUS_TEMPLATE_DIALOG_H_
#define JZ_MODBUS_TEMPLATE_DIALOG_H_

#include <QMainWindow>
#include <QStackedWidget>
#include "UiCommon.h"
#include "JZBaseDialog.h"
#include "JZSearchTreeWidget.h"
#include "jzmodbus/JZModbusParam.h"

//ModbusStargeDialog
class JZModbusTemplateDialog : public JZBaseDialog
{
    Q_OBJECT

public:
    JZModbusTemplateDialog(QWidget *widget = nullptr);    
    
    JZModbusConfig config();

protected:
    virtual bool onOk() override;
    void initConfig();

    JZModbusConfig m_config;
    QTreeWidget *m_tree;
};

#endif