#ifndef TEST_CAMERA_H_
#define TEST_CAMERA_H_

#include <QObject>
#include "../script/test_base.h"

class CameraTest : public BaseTest
{
    Q_OBJECT

public:
    CameraTest();

private slots:
    void testFile();

protected:


};

void test_camera(int argc, char *argv[]);

#endif // ! TEST_CAMERA_H_
