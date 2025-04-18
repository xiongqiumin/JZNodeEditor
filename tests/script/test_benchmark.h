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
    void testTryCatch();
    
protected:
    class Benchmark
    {
    public:
        Benchmark();

        inline qint64 step() const{ return m_step; }

        void reset(QString name);
        bool run();
        void clear();
        void report();

    protected:
        struct RunInfo
        {
            QString name;
            qint64 count;
            qint64 time;
        };

        QList<RunInfo> m_runInfo;
        qint64 m_stepStart;
        QElapsedTimer m_timer;
        qint64 m_step;
        qint64 m_count;
        bool m_first;
        QString m_name;
    };

    Benchmark m_benchmark;
};

void test_benchmark(int argc, char *argv[]);

#endif