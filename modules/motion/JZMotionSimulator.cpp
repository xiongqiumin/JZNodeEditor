#include "JZMotionSimulator.h"
#include <QDebug>
#include <QDateTime>

JZMotionSimulator::JZMotionSimulator(QObject* parent) : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &JZMotionSimulator::onTimerTick);

    m_currentProgram = nullptr;
    m_currentInstIndex = 0;
    m_currentX = 0.0;
    m_currentY = 0.0;
    m_currentZ = 0.0;
    m_currentF = 100;
    m_maxF = 10000;
    m_isSimulating = false;
    m_simulationStep = 0.1; // 默认步长0.1mm
    m_speedMultiplier = 100; // 默认正常速度
    m_timeGap = 20;

    m_totalRunningTime = 0.0;
    m_currentRunningTime = 0.0;
    m_lastTickTime = QDateTime::currentMSecsSinceEpoch();
}

JZMotionSimulator::~JZMotionSimulator() {}

void JZMotionSimulator::setSpeedMultiplier(int multiplier)
{
    // 确保倍率大于0
    m_speedMultiplier = multiplier;
}

void JZMotionSimulator::run(const JZGCodeProgram* gcode)
{
    if (m_isSimulating) {
        stopSimulation();
    }

    m_currentProgram = gcode;
    m_currentInstIndex = 0;
    m_currentX = 0.0;
    m_currentY = 0.0;
    m_currentZ = 0.0;
    m_currentF = 0.0;
    m_isSimulating = true;

    // 计算总运行时间
    m_totalRunningTime = 0.0;
    for (const auto& inst : gcode->instList) {
        m_totalRunningTime += calculateInstructionTime(inst);
    }

    emit sigPos(m_currentX, m_currentY, m_currentZ);
    emit sigTimeUpdated(0.0, m_totalRunningTime);

    // 开始处理第一条指令
    processNextInstruction();
}

void JZMotionSimulator::pause()
{
    m_timer->stop();
}

void JZMotionSimulator::resume()
{
    m_timer->start(m_timeGap);
}

void JZMotionSimulator::stopSimulation()
{
    m_timer->stop();
    m_isSimulating = false;
    emit sigSimulationFinished();
}

void JZMotionSimulator::onTimerTick()
{
    if (!m_isSimulating || !m_currentProgram) {
        m_timer->stop();
        return;
    }

    // 计算时间增量（考虑速度倍率）
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    m_lastTickTime = currentTime;

    if (m_currentInstIndex >= m_currentProgram->instList.size()) {
        stopSimulation();
        return;
    }

    const JZGCodeInstance& inst = m_currentProgram->instList[m_currentInstIndex];

    switch (inst.type) {
    case JZGCodeInstance::G0:
    case JZGCodeInstance::G1:
        executeLinearStep();
        break;

    case JZGCodeInstance::G2:
    case JZGCodeInstance::G3:
        executeArcStep();
        break;

    case JZGCodeInstance::M4:
    case JZGCodeInstance::M5:
        // 主轴控制指令立即执行
        qDebug() << "执行主轴控制指令:" << (inst.type == JZGCodeInstance::M4 ? "M4" : "M5");
        m_currentInstIndex++;
        processNextInstruction();
        break;

    default:
        m_currentInstIndex++;
        processNextInstruction();
        break;
    }

    emit sigTimeUpdated(m_currentRunningTime, m_totalRunningTime);
}

void JZMotionSimulator::processNextInstruction()
{
    if (!m_isSimulating || !m_currentProgram) return;

    if (m_currentInstIndex >= m_currentProgram->instList.size()) {
        stopSimulation();
        return;
    }

    const JZGCodeInstance& inst = m_currentProgram->instList[m_currentInstIndex];

    // 更新当前进给速度
    if (inst.F > 0) {
        m_currentF = inst.F; // mm/min
    }

    // 根据指令类型准备执行
    switch (inst.type) {
    case JZGCodeInstance::G0:
    case JZGCodeInstance::G1:
        executeLinearMotion(inst);
        break;

    case JZGCodeInstance::G2:
    case JZGCodeInstance::G3:
        executeArcMotion(inst);
        break;

    default:
        // 其他指令立即执行
        m_currentInstIndex++;
        processNextInstruction();
        break;
    }

    m_timer->start(m_timeGap);
}

void JZMotionSimulator::executeLinearMotion(const JZGCodeInstance& inst)
{
    // 目标位置
    m_linearInfo.targetX = inst.X;
    m_linearInfo.targetY = inst.Y;
    m_linearInfo.targetZ = inst.Z;

    // 如果指令中未指定坐标，则使用当前坐标
    if (std::isnan(m_linearInfo.targetX)) m_linearInfo.targetX = m_currentX;
    if (std::isnan(m_linearInfo.targetY)) m_linearInfo.targetY = m_currentY;
    if (std::isnan(m_linearInfo.targetZ)) m_linearInfo.targetZ = m_currentZ;

    // 计算总距离
    m_linearInfo.totalDistance = std::hypot(
        m_linearInfo.targetX - m_currentX,
        m_linearInfo.targetY - m_currentY,
        m_linearInfo.targetZ - m_currentZ
    );

    m_linearInfo.remainingDistance = m_linearInfo.totalDistance;

    qDebug() << "执行线性运动:"
        << "起点:" << m_currentX << "," << m_currentY << "," << m_currentZ
        << "终点:" << m_linearInfo.targetX << "," << m_linearInfo.targetY << "," << m_linearInfo.targetZ
        << "距离:" << m_linearInfo.totalDistance
        << "速度:" << m_currentF;

    // 立即执行第一步
    executeLinearStep();
}

void JZMotionSimulator::executeLinearStep()
{
    if (m_linearInfo.remainingDistance <= 0) {
        // 运动完成
        m_currentX = m_linearInfo.targetX;
        m_currentY = m_linearInfo.targetY;
        m_currentZ = m_linearInfo.targetZ;

        emit sigPos(m_currentX, m_currentY, m_currentZ);
        m_currentInstIndex++;
        processNextInstruction();
        return;
    }

    const JZGCodeInstance& inst = m_currentProgram->instList[m_currentInstIndex];
    // 计算步长（考虑速度倍率）
    double feedRate = m_currentF; // mm/min
    if (inst.type == JZGCodeInstance::G0)
        feedRate = m_maxF;

    double stepSize = m_speedMultiplier * feedRate * (m_timeGap / 60000.0); // 基于10ms间隔计算步长
    // 确保不超过剩余距离
    if (stepSize > m_linearInfo.remainingDistance) {
        stepSize = m_linearInfo.remainingDistance;
    }

    // 计算移动方向
    double dx = m_linearInfo.targetX - m_currentX;
    double dy = m_linearInfo.targetY - m_currentY;
    double dz = m_linearInfo.targetZ - m_currentZ;
    double distance = std::hypot(dx, dy, dz);

    if (distance > 0) {
        // 计算单位向量
        double unitX = dx / distance;
        double unitY = dy / distance;
        double unitZ = dz / distance;

        // 计算新位置
        m_currentX += unitX * stepSize;
        m_currentY += unitY * stepSize;
        m_currentZ += unitZ * stepSize;

        // 更新剩余距离
        m_linearInfo.remainingDistance -= stepSize;
    }

    // 发送位置更新
    emit sigPos(m_currentX, m_currentY, m_currentZ);
}

void JZMotionSimulator::executeArcMotion(const JZGCodeInstance& inst)
{
    // 当前位置作为起点
    double startX = m_currentX;
    double startY = m_currentY;
    double startZ = m_currentZ;

    // 目标位置
    double endX = inst.X;
    double endY = inst.Y;
    double endZ = inst.Z;

    // 如果指令中未指定终点坐标，则使用当前坐标
    if (std::isnan(endX)) endX = startX;
    if (std::isnan(endY)) endY = startY;
    if (std::isnan(endZ)) endZ = startZ;

    // 圆弧方向
    bool isClockwise = (inst.type == JZGCodeInstance::G2);

    // 计算圆心
    double centerX, centerY, centerZ;
    if (inst.R != 0.0) {
        // 使用R参数计算圆心
        if (!calculateCenterByRadius(startX, startY, endX, endY, inst.R, isClockwise, &centerX, &centerY)) {
            qDebug() << "错误: 无法根据R参数计算圆心";
            m_currentInstIndex++;
            processNextInstruction();
            return;
        }
        centerZ = startZ; // 假设Z坐标不变
    }
    else {
        // 使用I/J/K参数计算圆心（相对偏移量）
        centerX = startX + inst.I;
        centerY = startY + inst.J;
        centerZ = startZ + inst.K;
    }

    // 计算圆弧半径
    double radiusX = std::hypot(startX - centerX, startY - centerY);
    double radiusY = std::hypot(endX - centerX, endY - centerY);

    // 检查半径是否一致（误差范围内）
    if (std::abs(radiusX - radiusY) > 0.001) {
        qDebug() << "错误: 圆弧起点和终点到圆心的距离不一致";
        m_currentInstIndex++;
        processNextInstruction();
        return;
    }

    // 计算起点和终点角度
    double startAngle = std::atan2(startY - centerY, startX - centerX);
    double endAngle = std::atan2(endY - centerY, endX - centerX);

    // 规范化角度到 [0, 2π)
    startAngle = normalizeAngle(startAngle);
    endAngle = normalizeAngle(endAngle);

    // 计算旋转角度（考虑顺逆时针）
    double sweepAngle;
    if (isClockwise) {
        sweepAngle = startAngle - endAngle;
        if (sweepAngle < 0) sweepAngle += 2 * M_PI;
    }
    else {
        sweepAngle = endAngle - startAngle;
        if (sweepAngle < 0) sweepAngle += 2 * M_PI;
    }

    // 计算圆弧长度
    double arcLength = radiusX * sweepAngle;

    // 圆弧路径信息
    qDebug() << "圆弧信息:"
        << "起点:" << startX << "," << startY
        << "终点:" << endX << "," << endY
        << "圆心:" << centerX << "," << centerY
        << "半径:" << radiusX
        << "方向:" << (isClockwise ? "顺时针" : "逆时针")
        << "角度范围:" << startAngle * 180 / M_PI << "->" << endAngle * 180 / M_PI
        << "旋转角度:" << sweepAngle * 180 / M_PI
        << "弧长:" << arcLength;

    // 准备圆弧插补
    m_arcInfo.centerX = centerX;
    m_arcInfo.centerY = centerY;
    m_arcInfo.startAngle = startAngle;
    m_arcInfo.sweepAngle = sweepAngle;
    m_arcInfo.radius = radiusX;
    m_arcInfo.isClockwise = isClockwise;
    m_arcInfo.currentAngle = startAngle;
    m_arcInfo.totalArcLength = arcLength;
    m_arcInfo.remainingArcLength = arcLength;
    m_arcInfo.targetZ = endZ;
    m_arcInfo.zStep = (endZ - startZ) * (10.0 / 60000.0) * m_currentF / arcLength;

    // 执行第一步
    executeArcStep();
}

void JZMotionSimulator::executeArcStep()
{
    if (m_arcInfo.remainingArcLength <= 0) {
        // 圆弧完成
        m_currentX = m_arcInfo.centerX + m_arcInfo.radius * std::cos(m_arcInfo.startAngle + m_arcInfo.sweepAngle);
        m_currentY = m_arcInfo.centerY + m_arcInfo.radius * std::sin(m_arcInfo.startAngle + m_arcInfo.sweepAngle);
        m_currentZ = m_arcInfo.targetZ;

        emit sigPos(m_currentX, m_currentY, m_currentZ);
        m_currentInstIndex++;
        processNextInstruction();
        return;
    }

    // 计算步长（考虑速度倍率）
    double feedRate = m_currentF; // mm/min
    double stepSize = m_speedMultiplier * feedRate * (m_timeGap / 60000.0); // 基于10ms间隔计算步长

    // 确保不超过剩余弧长
    if (stepSize > m_arcInfo.remainingArcLength) {
        stepSize = m_arcInfo.remainingArcLength;
    }

    // 计算角度增量
    double angleIncrement = stepSize / m_arcInfo.radius;

    // 更新当前角度
    if (m_arcInfo.isClockwise) {
        m_arcInfo.currentAngle -= angleIncrement;
    }
    else {
        m_arcInfo.currentAngle += angleIncrement;
    }

    // 规范化角度
    m_arcInfo.currentAngle = normalizeAngle(m_arcInfo.currentAngle);

    // 计算当前位置
    m_currentX = m_arcInfo.centerX + m_arcInfo.radius * std::cos(m_arcInfo.currentAngle);
    m_currentY = m_arcInfo.centerY + m_arcInfo.radius * std::sin(m_arcInfo.currentAngle);
    m_currentZ += m_arcInfo.zStep;

    // 更新剩余弧长
    m_arcInfo.remainingArcLength -= stepSize;

    // 发送位置更新
    emit sigPos(m_currentX, m_currentY, m_currentZ);
}

bool JZMotionSimulator::calculateCenterByRadius(double x1, double y1, double x2, double y2, double radius, bool isClockwise, double* centerX, double* centerY) const
{
    // 计算起点和终点之间的距离
    double dx = x2 - x1;
    double dy = y2 - y1;
    double distance = std::hypot(dx, dy);

    // 检查是否可以形成圆弧
    if (distance > 2 * radius) {
        qDebug() << "错误: 两点之间的距离大于直径，无法形成圆弧";
        return false;
    }

    // 计算中点
    double midX = (x1 + x2) / 2;
    double midY = (y1 + y2) / 2;

    // 计算从起点到终点的方向角度
    double angle = std::atan2(dy, dx);

    // 计算从中点到圆心的距离
    double h = std::sqrt(radius * radius - (distance / 2) * (distance / 2));

    // 计算圆心位置（两种可能的圆心）
    double centerX1 = midX + h * std::sin(angle);
    double centerY1 = midY - h * std::cos(angle);

    double centerX2 = midX - h * std::sin(angle);
    double centerY2 = midY + h * std::cos(angle);

    // 简化处理，选择第一个计算的圆心
    *centerX = centerX1;
    *centerY = centerY1;

    return true;
}

double JZMotionSimulator::normalizeAngle(double angle) const
{
    // 将角度规范化到 [0, 2π) 范围
    while (angle < 0) angle += 2 * M_PI;
    while (angle >= 2 * M_PI) angle -= 2 * M_PI;
    return angle;
}

double JZMotionSimulator::calculateInstructionTime(const JZGCodeInstance& inst) const
{
    double feedRate = inst.F > 0 ? inst.F : m_currentF;
    if (feedRate <= 0) return 0.0; // 避免除零错误

    double distance = 0.0;

    switch (inst.type) {
    case JZGCodeInstance::G0:
    case JZGCodeInstance::G1:
        distance = calculateLinearDistance(inst);
        break;

    case JZGCodeInstance::G2:
    case JZGCodeInstance::G3:
        distance = calculateArcLength(inst);
        break;

    default:
        // 非运动指令时间忽略不计
        return 0.0;
    }

    // 计算时间（分钟）并转换为秒
    return (distance / feedRate) * 60.0;
}

double JZMotionSimulator::calculateLinearDistance(const JZGCodeInstance& inst) const
{
    // 假设当前位置为起点
    double startX = m_currentX;
    double startY = m_currentY;
    double startZ = m_currentZ;

    // 目标位置
    double endX = inst.X;
    double endY = inst.Y;
    double endZ = inst.Z;

    // 如果指令中未指定坐标，则使用当前坐标
    if (std::isnan(endX)) endX = startX;
    if (std::isnan(endY)) endY = startY;
    if (std::isnan(endZ)) endZ = startZ;

    // 计算直线距离
    return std::hypot(endX - startX, endY - startY, endZ - startZ);
}

double JZMotionSimulator::calculateArcLength(const JZGCodeInstance& inst) const
{
    // 假设当前位置为起点
    double startX = m_currentX;
    double startY = m_currentY;
    double startZ = m_currentZ;

    // 目标位置
    double endX = inst.X;
    double endY = inst.Y;
    double endZ = inst.Z;

    // 如果指令中未指定终点坐标，则使用当前坐标
    if (std::isnan(endX)) endX = startX;
    if (std::isnan(endY)) endY = startY;
    if (std::isnan(endZ)) endZ = startZ;

    // 圆弧方向
    bool isClockwise = (inst.type == JZGCodeInstance::G2);

    // 计算圆心
    double centerX, centerY;
    if (inst.R != 0.0) {
        // 使用R参数计算圆心（简化版）
        if (!calculateCenterByRadius(startX, startY, endX, endY, inst.R, isClockwise, &centerX, &centerY)) {
            return 0.0;
        }
    }
    else {
        // 使用I/J/K参数计算圆心
        centerX = startX + inst.I;
        centerY = startY + inst.J;
    }

    // 计算圆弧半径
    double radius = std::hypot(startX - centerX, startY - centerY);

    // 计算起点和终点角度
    double startAngle = std::atan2(startY - centerY, startX - centerX);
    double endAngle = std::atan2(endY - centerY, endX - centerX);

    // 规范化角度到 [0, 2π)
    startAngle = normalizeAngle(startAngle);
    endAngle = normalizeAngle(endAngle);

    // 计算旋转角度
    double sweepAngle;
    if (isClockwise) {
        sweepAngle = startAngle - endAngle;
        if (sweepAngle < 0) sweepAngle += 2 * M_PI;
    }
    else {
        sweepAngle = endAngle - startAngle;
        if (sweepAngle < 0) sweepAngle += 2 * M_PI;
    }

    // 计算圆弧长度
    return radius * sweepAngle;
}

double JZMotionSimulator::getTotalRunningTime() const
{
    return m_totalRunningTime;
}

double JZMotionSimulator::getCurrentRunningTime() const
{
    return m_currentRunningTime;
}

double JZMotionSimulator::getRemainingTime() const
{
    return qMax(0.0, m_totalRunningTime - m_currentRunningTime);
}