#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QQueue>
#include <QMap>
#include <QMutex>
#include <QDataStream>
#include "../JZMotionDriver.h"

namespace motean
{

// 定义消息类型
enum class MessageType : uint16_t {
    // 连接相关
    ConnectRequest = 0x0F,
    ConnectResponse = 0x8F,
    
    // AD相关
    ADRequest = 0x02,
    ADResponse = 0x82,
    
    // DA相关
    DARequest = 0x04,
    DAResponse = 0x84,
    
    // 输入IO相关
    InputIORequest = 0x07,
    InputIOResponse = 0x87,
    
    // 输出IO相关
    OutputIORequest = 0x0A,
    OutputIOResponse = 0x8A,
    
    // 伺服电机配置相关
    ServoConfigRequest = 0x0B,
    ServoConfigResponse = 0x8B,
    
    // 伺服电机位置设置相关
    ServoPositionRequest = 0x0C,
    ServoPositionResponse = 0x8C,
    
    // 伺服电机状态相关
    ServoStatusRequest = 0x0D,
    ServoStatusResponse = 0x8D,
    
    // 伺服电机命令相关
    ServoCommandRequest = 0x0E,
    ServoCommandResponse = 0x8E,
    
    // 直线运动命令相关
    LinearMotionRequest = 0x11,
    LinearMotionResponse = 0x91,
    
    // 圆弧运动命令相关
    ArcMotionRequest = 0x12,
    ArcMotionResponse = 0x92,
    
    // 插补运动状态相关
    InterpolationStatusRequest = 0x10,
    InterpolationStatusResponse = 0x90
};

// 基类消息结构
struct Message 
{
    uint16_t head;
    uint16_t board;
    uint16_t end;
};

// 连接消息
struct ConnectMessage : public Message {
    ConnectMessage(bool isRequest) 
        : Message(isRequest ? MessageType::ConnectRequest : MessageType::ConnectResponse) {}
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// AD消息
struct ADMessage : public Message {
    uint16_t no;        // AD通道号
    uint16_t serial;    // 流水号
    uint16_t ad;        // AD采样值
    uint16_t change_flag; // 变化标志
    
    ADMessage(bool isRequest);
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// DA消息
struct DAMessage : public Message {
    uint16_t no;        // DA通道号
    uint16_t serial;    // 流水号
    uint16_t da;        // DA输出值
    uint16_t change_flag; // 变化标志
    
    DAMessage(bool isRequest);
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 输入IO消息
struct InputIOMessage : public Message {
    uint16_t serial;    // 流水号
    uint8_t input[8];   // 对应的64个输入口
    uint16_t change_flag; // 变化标志
    
    InputIOMessage(bool isRequest) 
        : Message(isRequest ? MessageType::InputIORequest : MessageType::InputIOResponse), 
          serial(0), change_flag(0x01) {
        memset(input, 0, sizeof(input));
    }
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 输出IO消息
struct OutputIOMessage : public Message {
    uint16_t serial;    // 流水号
    uint8_t output[8];  // 对应的64个输出口
    uint16_t change_flag; // 变化标志
    
    OutputIOMessage(bool isRequest) 
        : Message(isRequest ? MessageType::OutputIORequest : MessageType::OutputIOResponse), 
          serial(0), change_flag(0x01) {
        memset(output, 0, sizeof(output));
    }
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 伺服电机配置消息
struct ServoConfigMessage : public Message {
    uint16_t no;            // 伺服电机号
    uint16_t serial;        // 流水号
    uint16_t init;          // 初始化标志
    int16_t motor_sw;       // 电机使能IO口号
    int16_t motor_alarm;    // 电机报警IO口号
    int16_t motor_direction_true; // 电机方向
    int16_t motor_checkmin; // 电机最小限位IO号
    int16_t motor_checkmax; // 电机最大IO号
    int32_t motor_pomin;    // 电机最小位置
    int32_t motor_pomax;    // 电机最大位置
    uint16_t change_flag;   // 变化标志
    
    ServoConfigMessage();
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 伺服电机位置消息
struct ServoPositionMessage : public Message {
    uint16_t no;            // 伺服电机号
    uint16_t serial;        // 流水号
    int32_t motor_po;       // 电机位置
    uint16_t change_flag;   // 变化标志
    
    ServoPositionMessage(bool isRequest);
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 伺服电机状态消息
struct ServoStatusMessage : public Message {
    uint16_t no;            // 伺服电机号
    uint16_t serial;        // 流水号
    uint16_t motor_status;  // 电机状态
    uint16_t motor_mux;     // 电机复用状态
    uint16_t motor_checkmin; // 最小限位状态
    uint16_t motor_checkmax; // 最大限位状态
    uint16_t motor_sw;      // 电机开关状态
    uint16_t motor_alarm;   // 电机报警状态
    uint16_t motor_direction; // 电机方向
    int32_t motor_po;       // 电机当前位置
    uint16_t change_flag;   // 变化标志
    
    ServoStatusMessage(bool isRequest);
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 伺服电机命令消息
struct ServoCommandMessage : public Message {
    uint16_t no;            // 伺服电机号
    uint16_t serial;        // 流水号
    uint16_t motor_fun;     // 电机功能
    int32_t motor_stepnum;  // 总步数
    uint32_t motor_sp;      // 电机速度
    uint16_t motor_sp_slee; // 电机休眠速度
    uint16_t motor_st_time; // 起始加速时间
    uint16_t motor_end_time; // 结束减速时间
    uint16_t motor_checkmin; // 最小限位控制
    uint16_t motor_checkmax; // 最大限位控制
    uint16_t motor_pomin;   // 最小位置控制
    uint16_t motor_pomax;   // 最大位置控制
    uint16_t change_flag;   // 变化标志
    
    ServoCommandMessage(bool isRequest);
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 直线运动命令消息
struct LinearMotionMessage : public Message {
    uint16_t serial;        // 流水号
    uint16_t fun;           // 功能
    uint16_t motor_num;     // 电机数量
    uint16_t xn;            // X轴号
    uint16_t yn;            // Y轴号
    uint16_t zn;            // Z轴号
    int32_t x;              // X终点坐标
    int32_t y;              // Y终点坐标
    int32_t z;              // Z终点坐标
    uint16_t sp;            // 速度
    uint16_t acc;           // 加速平滑度
    uint16_t dec;           // 减速平滑度
    uint16_t change_flag;   // 变化标志
    
    LinearMotionMessage(bool isRequest);
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 圆弧运动命令消息
struct ArcMotionMessage : public Message {
    uint16_t serial;        // 流水号
    uint16_t fun;           // 功能
    uint16_t xn;            // X轴号
    uint16_t yn;            // Y轴号
    int32_t x;              // X终点坐标
    int32_t y;              // Y终点坐标
    int32_t r;              // 圆半径
    uint16_t s;             // 圆方向
    uint16_t jg;            // 采样间隔
    uint16_t sp;            // 速度
    uint16_t acc;           // 加速平滑度
    uint16_t dec;           // 减速平滑度
    uint16_t change_flag;   // 变化标志
    
    ArcMotionMessage(bool isRequest);
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 插补运动状态消息
struct InterpolationStatusMessage : public Message {
    uint16_t serial;        // 流水号
    uint16_t status;        // 状态
    uint16_t change_flag;   // 变化标志
    
    InterpolationStatusMessage(bool isRequest);
    
    QByteArray toByteArray() const override;
    bool fromByteArray(const QByteArray& data) override;
};

// 通信管理器类
class JZMotionMoteanDriver : public JZMotionDriver 
{
    Q_OBJECT
	
public:
    explicit JZMotionMoteanDriver(QObject *parent = nullptr);
    ~JZMotionMoteanDriver();
    
    bool open(const QString& portName);
    void close();
    bool isConnected() const;
    
    
	void sendGCode();
	
signals:
    
private slots:
    void onReadyRead();
    void onHandleError(QSerialPort::SerialPortError error);
    void onHandleTimeout();
    
private:
    // 发送消息的辅助函数
    void sendMessage(const Message& message);
    void resendMessage();
    
    // 解析接收到的数据
    void parseData(const QByteArray& data);
    
    // 流水号管理
    uint16_t getNextSerialNumber();
    
    QSerialPort *m_serialPort;
};

}

#endif // PROTOCOL_H    