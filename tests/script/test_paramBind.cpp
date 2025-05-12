#include <QTest>
#include <math.h>
#include <functional>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QSlider>
#include "test_paramBind.h"
#include "mvvm/JZNodeVariableBind.h"

//ParamBindTest
ParamBindTest::ParamBindTest()
{

}

void ParamBindTest::testSample()
{
    auto class_item = m_file->addClass("TestClass");
    class_item->addMemberVariable("a", Type_int);

    m_project.registType();
    auto env = m_project.environment();

    JZNodeObject *jz_obj = env->objectManager()->create("TestClass");
    JZNodeObjectPointer ptr(jz_obj, true);

    QWidget* w = new QWidget();
    w->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* l = new QVBoxLayout(w);
    QLineEdit* l1 = new QLineEdit();
    QLineEdit* l2 = new QLineEdit();
    QLabel* label = new QLabel();
    QSlider* slider = new QSlider();
    slider->setRange(0, 1000000);
    l->addWidget(l1);
    l->addWidget(l2);
    l->addWidget(label);
    l->addWidget(slider);
    w->show();

    JZBindManager::instance()->bind(l1, jz_obj, "a", Dir_duplex);
    JZBindManager::instance()->bind(l2, jz_obj, "a", Dir_duplex);
    JZBindManager::instance()->bind(label, jz_obj, "a", Dir_duplex);
    JZBindManager::instance()->bind(slider, jz_obj, "a", Dir_duplex);
    
    QTest::qWaitForWindowActive(w);
    jz_obj->setParam("a", 100);
    QCOMPARE(l1->text().toInt(), 100);
    QCOMPARE(l2->text().toInt(), 100);
    QCOMPARE(label->text().toInt(), 100);
    QCOMPARE(slider->value(), 100);

    l1->setText("500");
    QCOMPARE(jz_obj->param("a").toInt(), 500);
    QCOMPARE(l2->text().toInt(), 500);
    QCOMPARE(label->text().toInt(), 500);
    QCOMPARE(slider->value(), 500);
}

void test_paramBind(int argc, char* argv[])
{
    ParamBindTest test;
    QTest::qExec(&test, argc, argv);
}