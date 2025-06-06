#ifndef JZMOTIONSIMULATOR_H
#define JZMOTIONSIMULATOR_H

#include <QObject>
#include <QList>
#include <QString>
#include <QTimer>
#include <cmath>
#include "JZGCodeParser.h"

class JZMotionSimulator : public QObject
{
    Q_OBJECT

public:
    explicit JZMotionSimulator(QObject* parent = nullptr);
    ~JZMotionSimulator();

    void run(const JZGCodeProgram *gcode);
    void pause();
    void resume();
    void stopSimulation();
    void setSimulationStep(double step) { m_simulationStep = step; }
    void setSpeedMultiplier(int multiplier); // 设置模拟速度倍率

    double getTotalRunningTime() const; // 获取预计总运行时间(秒)
    double getCurrentRunningTime() const; // 获取当前已运行时间(秒)
    double getRemainingTime() const; // 获取预计剩余时间(秒)

signals:
    void sigPos(double x, double y, double z);
    void sigTimeUpdated(double currentTime, double totalTime); // 时间更新信号
    void sigSimulationFinished();

protected slots:
    void onTimerTick();

private:
    void processNextInstruction();
    void executeLinearMotion(const JZGCodeInstance& inst);
    void executeArcMotion(const JZGCodeInstance& inst);
    void executeLinearStep();
    void executeArcStep();

    bool calculateCenterByRadius(double x1, double y1, double x2, double y2, double radius, bool isClockwise, double* centerX, double* centerY) const;
    double normalizeAngle(double angle) const;

    // 计算指令运行时间
    double calculateInstructionTime(const JZGCodeInstance& inst) const;
    double calculateLinearDistance(const JZGCodeInstance& inst) const;
    double calculateArcLength(const JZGCodeInstance& inst) const;

    QTimer* m_timer;
    const JZGCodeProgram* m_currentProgram;
    int m_currentInstIndex;

    // 当前状态
    double m_currentX;
    double m_currentY;
    double m_currentZ;
    double m_currentF;       // 当前进给速度(mm/min)
    bool m_isSimulating;
    double m_simulationStep; // 模拟步长(mm)
    int m_speedMultiplier; // 模拟速度倍率(1.0=正常速度)
    double m_maxF;

    // 线性运动信息
    struct {
        double targetX;
        double targetY;
        double targetZ;
        double totalDistance;
        double remainingDistance;
    } m_linearInfo;

    // 圆弧插补信息
    struct {
        double centerX;
        double centerY;
        double startAngle;
        double sweepAngle;
        double radius;
        bool isClockwise;
        double currentAngle;
        double totalArcLength;
        double remainingArcLength;
        double targetZ;
        double zStep;
    } m_arcInfo;

    // 时间跟踪
    double m_totalRunningTime; // 总运行时间(秒)
    double m_currentRunningTime; // 当前运行时间(秒)
    double m_lastTickTime; // 上次定时器触发时间(毫秒)
    int m_timeGap;
};

#endif // JZMOTIONSIMULATOR_H    