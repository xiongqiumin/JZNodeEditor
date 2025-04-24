#ifndef TEST_EVENT_H_
#define TEST_EVENT_H_

#include <QObject>
#include "test_base.h"

class EventTest : public BaseTest
{
    Q_OBJECT

public:
    EventTest();

private slots:
    void testTimer();
    void testMultiTimer();

protected:

};

void test_event(int argc, char *argv[]);

#endif
