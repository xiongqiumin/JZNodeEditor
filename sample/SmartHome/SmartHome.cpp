#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include "SmartHome.h"
#include "JZNodeBuilder.h"
#include "JZNodeVM.h"
#include "UiCommon.h"
#include "JZNodeFunction.h"
#include "JZNodeValue.h"
#include "JZUiFile.h"
#include "JZNodeView.h"
#include "JZNodeUtils.h"

SampleSmartHome::SampleSmartHome()
{
    newProject("SmartHome");        

    JZUiFile *ui_file = dynamic_cast<JZUiFile*>(m_project.getItem("./mainwindow.ui"));
    ui_file->setXml(loadUi("SmartHome.ui"));
    m_project.saveItem(ui_file);

    addResources("Resources");

    addEvent();
}

SampleSmartHome::~SampleSmartHome()
{

}

void SampleSmartHome::addEvent()
{   
    auto btn_meta = JZNodeObjectManager::instance()->meta("PushButton");
    auto meta = JZNodeObjectManager::instance()->meta("MainWindow");

    auto class_file = m_project.getClass("MainWindow");
    auto script = class_file->flow("ÊÂ¼þ");

    JZNodeParam *stack = new JZNodeParam();
    stack->setVariable("stackedWidget");

    JZNodeFunction *func1 = new JZNodeFunction();
    JZNodeFunction *func2 = new JZNodeFunction();
    JZNodeFunction *func3 = new JZNodeFunction();
    JZNodeFunction *func4 = new JZNodeFunction();

    func1->setFunction("StackedWidget.setCurrentIndex");
    func2->setFunction("StackedWidget.setCurrentIndex");
    func3->setFunction("StackedWidget.setCurrentIndex");
    func4->setFunction("StackedWidget.setCurrentIndex");

    func1->setParamInValue(1, "1");
    func2->setParamInValue(1, "2");
    func3->setParamInValue(1, "0");
    func4->setParamInValue(1, "0");

    script->addNode(stack);
    script->addNode(func1);
    script->addNode(func2);
    script->addNode(func3);
    script->addNode(func4);

    script->addConnect(stack->paramOutGemo(0), func1->paramInGemo(0));
    script->addConnect(stack->paramOutGemo(0), func2->paramInGemo(0));
    script->addConnect(stack->paramOutGemo(0), func3->paramInGemo(0));
    script->addConnect(stack->paramOutGemo(0), func4->paramInGemo(0));

    //btnStart
    JZNodeSingleEvent *btn1 = new JZNodeSingleEvent();
    btn1->setSingle(btn_meta->single("clicked"));
    btn1->setVariable("this.living_room_btn");

    JZNodeSingleEvent *btn2 = new JZNodeSingleEvent();
    btn2->setSingle(btn_meta->single("clicked"));
    btn2->setVariable("this.coffee_btn");

    JZNodeSingleEvent *btn3 = new JZNodeSingleEvent();
    btn3->setSingle(btn_meta->single("clicked"));
    btn3->setVariable("this.close1");

    JZNodeSingleEvent *btn4 = new JZNodeSingleEvent();
    btn4->setSingle(btn_meta->single("clicked"));
    btn4->setVariable("this.close2");

    script->addNode(btn1);
    script->addNode(btn2);
    script->addNode(btn3);
    script->addNode(btn4);

    script->addConnect(btn1->flowOutGemo(0), func1->flowInGemo());
    script->addConnect(btn2->flowOutGemo(0), func2->flowInGemo());
    script->addConnect(btn3->flowOutGemo(0), func3->flowInGemo());
    script->addConnect(btn4->flowOutGemo(0), func4->flowInGemo());
}