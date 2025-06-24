#ifndef JZ_NET_SIMULATOR_H_
#define JZ_NET_SIMULATOR_H_

#include <QTableWidget>
#include <QToolButton>
#include "../JZCommSimulatorWidget.h"

class JZModBusSimulatorConfig
{
public: 
};

class JZModBusSimulator : public JZCommSimulatorWidget
{
    Q_OBJECT

public:
    JZModBusSimulator();
    ~JZModBusSimulator();

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;
    virtual void setConfig(const QByteArray &buffer) override;
    virtual QByteArray getConfig() override;
    
};








#endif