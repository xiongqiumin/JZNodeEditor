#ifndef JZ_COMM_SIMULATOR_WIDGET_H_
#define JZ_COMM_SIMULATOR_WIDGET_H_

#include <QWidget>

enum JZCommSimulatorType 
{
    Sim_None,
    Sim_Modbus,
    Sim_Net,
    Sim_SerialPort,
};

class JZCommSimulatorWidget : public QWidget
{    
    Q_OBJECT

public:
    virtual JZCommSimulatorType type() = 0;
    virtual bool isOpen() = 0;
    virtual bool open() = 0;
    virtual void close() = 0;

    virtual void setConfig(const QByteArray &buffer) = 0;
    virtual QByteArray getConfig() = 0;
};

#endif // !JZ_COMM_SIMULATOR_WIDGET_H_
