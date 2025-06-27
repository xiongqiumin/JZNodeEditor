#include <QPainter>
#include <QVBoxLayout>
#include <QTimer>
#include "JZGPIOSimulator.h"

JZGPIOSimulatorConfig::JZGPIOSimulatorConfig()
{
}

QDataStream &operator<<(QDataStream &s, const JZGPIOSimulatorConfig &param)
{
    s << param.inCount << param.outCount;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZGPIOSimulatorConfig &param)
{
    s >> param.inCount >> param.outCount;
    return s;
}

// 单个IO引脚控件
GPIOPinWidget::GPIOPinWidget(PinType type, int index, QWidget *parent = nullptr)
    : QToolButton(parent)
    , m_type(type)
    , m_index(index)
    , m_state(0)
    , m_radius(18)
{
    m_timer = new QTimer(this);
    setFixedSize(60, 60);
    setToolTip(type == Input ? "Input Pin" : "Output Pin");
}

void GPIOPinWidget::setState(int state)
{
    if (m_state != state) {
        m_state = state;
        update();
    }
}

void GPIOPinWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), Qt::transparent);
    
    // 计算圆心位置
    QPoint center(width() / 2, height() / 2);
    
    // 根据类型和状态设置颜色
    QColor fillColor;
    if (m_type == Input) {
        fillColor = m_state ? QColor(0, 200, 0) : QColor(200, 200, 200);
    } else {
        fillColor = m_state ? QColor(200, 0, 0) : QColor(200, 200, 200);
    }
    
    // 绘制圆形
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(QBrush(fillColor));
    painter.drawEllipse(center, m_radius, m_radius);
    
    // 绘制引脚编号
    painter.setFont(QFont("Arial", 9));
    painter.drawText(rect(), Qt::AlignCenter, QString::number(m_index));
}

//JZGPIOSimulator
JZGPIOSimulator::JZGPIOSimulator()
{
    m_isOpen = false;
    setupUI();
}

JZGPIOSimulator::~JZGPIOSimulator()
{

}

bool JZGPIOSimulator::isOpen() 
{
    return m_isOpen;
}

bool JZGPIOSimulator::open() 
{
    m_isOpen = true;
    return true;
}

void JZGPIOSimulator::close() 
{
    m_isOpen = false;
}

void JZGPIOSimulator::setConfig(const QByteArray &buffer) 
{
    QDataStream stream(buffer);
    stream >> m_config;
    createPinWidgets();
}

QByteArray JZGPIOSimulator::getConfig() 
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << m_config;
    return buffer;
}

void JZGPIOSimulator::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 标题和状态
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    m_titleLabel = new QLabel("GPIO Simulator", this);
    m_titleLabel->setFont(QFont("Arial", 14, QFont::Bold));
    headerLayout->addWidget(m_titleLabel);
    
    m_statusLabel = new QLabel("Status: Closed", this);
    m_statusLabel->setFont(QFont("Arial", 10));
    m_statusLabel->setStyleSheet("color: red;");
    headerLayout->addStretch();
    headerLayout->addWidget(m_statusLabel);
    
    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(10);
    
    // 输入区域
    QLabel *inputLabel = new QLabel("Inputs:", this);
    mainLayout->addWidget(inputLabel);
    
    m_inputContainer = new QWidget(this);
    m_inputLayout = new QGridLayout(m_inputContainer);
    m_inputLayout->setSpacing(5);
    m_inputLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_inputContainer);
    
    // 输出区域
    QLabel *outputLabel = new QLabel("Outputs:", this);
    mainLayout->addWidget(outputLabel);
    
    m_outputContainer = new QWidget(this);
    m_outputLayout = new QGridLayout(m_outputContainer);
    m_outputLayout->setSpacing(5);
    m_outputLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_outputContainer);
    
    mainLayout->addStretch();
    
    // 创建IO控件
    createPinWidgets();
}

void JZGPIOSimulator::createPinWidgets()
{
    // 清除现有控件
    clearPinWidgets();
    
    // 创建输入控件
    for (int i = 0; i < m_config.inCount; ++i) {
        GPIOPinWidget *widget = new GPIOPinWidget(GPIOPinWidget::Input, i, this);
        m_inputWidgets.append(widget);
        m_inputLayout->addWidget(widget, i / 16, i % 16);
    }
    
    // 创建输出控件
    for (int i = 0; i < m_config.outCount; ++i) {
        GPIOPinWidget *widget = new GPIOPinWidget(GPIOPinWidget::Output, i, this);
        m_outputWidgets.append(widget);
        m_outputLayout->addWidget(widget, i / 16, i % 16);
    }
}

void JZGPIOSimulator::clearPinWidgets()
{
    // 清除输入控件
    for (auto widget : m_inputWidgets) {
        m_inputLayout->removeWidget(widget);
        delete widget;
    }
    m_inputWidgets.clear();
    
    // 清除输出控件
    for (auto widget : m_outputWidgets) {
        m_outputLayout->removeWidget(widget);
        delete widget;
    }
    m_outputWidgets.clear();
}