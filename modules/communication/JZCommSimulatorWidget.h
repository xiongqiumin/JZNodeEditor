#ifndef JZ_COMM_SIMULATOR_WIDGET_H_
#define JZ_COMM_SIMULATOR_WIDGET_H_

#include <QWidget>


class JZCommSimulatorWidget : public QWidget
{    
public:
    virtual bool isOpen() = 0;
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual void setting() = 0;
    virtual void setConfig(const QByteArray &buffer) = 0;
    virtual QByteArray getConfig() = 0;
};

#endif // !JZ_COMM_SIMULATOR_WIDGET_H_
