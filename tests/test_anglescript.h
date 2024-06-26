#ifndef TEST_ANGLE_SCRIPT_H_
#define TEST_ANGLE_SCRIPT_H_

#include <QObject>
#include "JZProject.h"
#include "JZNodeEngine.h"
#include "script/angelscript/as_tojzscript.h"
#include "test_base.h"

class AngleScriptTest : public BaseTest
{
    Q_OBJECT

public:
    AngleScriptTest();

private slots:
    void testHello();
    void testFab();
    
protected:
    bool buildAs(QString code);
};

#endif
