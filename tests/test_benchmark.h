#ifndef TEST_BENCH_MARK_H_
#define TEST_BENCH_MARK_H_

#include <QObject>
#include <QElapsedTimer>
#include "test_base.h"

class BenchmarkTest : public BaseTest
{
    Q_OBJECT

public:
    BenchmarkTest();

private slots:
    void testBase();
    void testCall();
    void testSort();
    void testSum();
    
protected:
    class Benchmark
    {
    public:
        Benchmark();

        inline int step() const{ return m_step; }

        void reset(QString name);
        bool run();
        void clear();
        void report();

    protected:
        struct RunInfo
        {
            QString name;
            int count;
            qint64 time;
        };

        QList<RunInfo> m_runInfo;
        qint64 m_stepStart;
        QElapsedTimer m_timer;
        int m_step;
        int m_count;
        bool m_first;
        QString m_name;
    };

    Benchmark m_benchmark;
};






#endif