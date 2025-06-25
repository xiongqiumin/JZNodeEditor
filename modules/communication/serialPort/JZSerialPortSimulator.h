#ifndef JZ_NET_SIMULATOR_H_
#define JZ_NET_SIMULATOR_H_

#include <QTableWidget>
#include <QToolButton>
#include "../JZCommSimulatorWidget.h"
#include "JZSerialPort.h"

//JZSerialPortConfig
class JZSerialPortConfig
{
public: 
    JZSerialPortConfig();
};
QDataStream &operator<<(QDataStream &s, const JZSerialPortConfig &param);
QDataStream &operator>>(QDataStream &s, JZSerialPortConfig &param);

//JZSerialPortSimulator
class JZSerialPortSimulator : public JZCommSimulatorWidget
{
    Q_OBJECT

public:
    JZSerialPortSimulator();
    ~JZSerialPortSimulator();

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;
    virtual void setConfig(const QByteArray &buffer) override;
    virtual QByteArray getConfig() override;
};








#endif