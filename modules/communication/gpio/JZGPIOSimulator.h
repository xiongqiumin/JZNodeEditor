#ifndef JZ_NET_SIMULATOR_H_
#define JZ_NET_SIMULATOR_H_

#include <QTableWidget>
#include <QToolButton>
#include <QLabel>
#include "../JZCommSimulatorWidget.h"

class JZGPIOSimulatorConfig
{
public: 
    JZGPIOSimulatorConfig();
	
	int inCount;
	int outCount;
};
QDataStream &operator<<(QDataStream &s, const JZGPIOSimulatorConfig &param);
QDataStream &operator>>(QDataStream &s, JZGPIOSimulatorConfig &param);


// 单个IO引脚控件
class GPIOPinWidget : public QToolButton
{
    Q_OBJECT

public:
    enum PinType { Input, Output };

    GPIOPinWidget(PinType type, int index, QWidget *parent = nullptr);
    void setState(int state);

    void paintEvent(QPaintEvent *event);

signals:

protected slots:
    void onClicked();

private:
    QTimer *m_timer;
    PinType m_type;
    int m_index;
    int m_state;
    int m_radius;
};

//JZGPIOSimulator
class JZGPIOSimulator : public JZCommSimulatorWidget
{
    Q_OBJECT

public:
    JZGPIOSimulator();
    ~JZGPIOSimulator();

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;
    virtual void setConfig(const QByteArray &buffer) override;
    virtual QByteArray getConfig() override;
    

protected:
    void setupUI();
    void createPinWidgets();
    void clearPinWidgets();

    bool m_isOpen;
    JZGPIOSimulatorConfig m_config;

    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QWidget *m_inputContainer;
    QWidget *m_outputContainer;
    QGridLayout *m_inputLayout;
    QGridLayout *m_outputLayout;
    QVector<GPIOPinWidget*> m_inputWidgets;
    QVector<GPIOPinWidget*> m_outputWidgets;
};








#endif