#ifndef TEST_TEST_PROJECT_H_
#define TEST_TEST_PROJECT_H_

#include <QObject>
#include "test_base.h"

class ProjectTest : public BaseTest
{
    Q_OBJECT

public:
    ProjectTest();

private slots:
    void saveLoad();

protected:

};

void test_project(int argc, char* argv[]);

#endif
