#ifndef TEST_DEBUG_H_
#define TEST_DEBUG_H_

#include <QObject>
#include "test_base.h"
#include "JZNodeDebugServer.h"
#include "JZNodeDebugClient.h"

class DebugTest : public BaseTest
{
    Q_OBJECT

public:
    DebugTest();

private slots:
    void testBreakPoint();
    void testDebugServer();
    void testBreakPointServer();

protected:
    virtual void clearTestCase() override;
    void startServer(QString func,QVariantList input);
    void stopServer();
    bool initWhileCase(QList<int> &id_list, QList<int> &value_list);

    JZNodeDebugServer m_server;
    JZNodeDebugClient m_client;
};

void test_debug(int argc, char *argv[]);

#endif
