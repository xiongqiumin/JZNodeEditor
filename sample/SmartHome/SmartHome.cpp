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
    auto btn_meta = m_objInst->meta("PushButton");
    auto meta = m_objInst->meta("MainWindow");

    QStringList btn_list = { "living_room_btn","coffee_btn","close1","close2"};
    QList<int> stack_index = {1,2,0,0};
    
    auto class_file = m_project.getClass("MainWindow");

    for(int i = 0; i < btn_list.size(); i++)
    {
        JZFunctionDefine define;
        define.name = "on_" + btn_list[i] + "_clicked";
        define.className = "MainWindow";
        define.isFlowFunction = true;
        define.paramIn.push_back(JZParamDefine("this", "MainWindow"));
        
        auto script = class_file->addMemberFunction(define); 
        
        JZNodeParam *stack = new JZNodeParam();
        stack->setVariable("stackedWidget");

        JZNodeFunction *func1 = new JZNodeFunction();
        func1->setFunction("QStackedWidget.setCurrentIndex");
        func1->setParamInValue(1, QString::number(stack_index[i]));

        script->addNode(stack);
        script->addNode(func1);
        script->addConnect(stack->paramOutGemo(0), func1->paramInGemo(0));

        auto node_start = script->getNode(0);
        script->addConnect(node_start->flowOutGemo(0), func1->flowInGemo());
    }
}