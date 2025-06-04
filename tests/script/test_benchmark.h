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
    Benchmark m_benchmark;
};

void test_benchmark(int argc, char *argv[]);

#endif