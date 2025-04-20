#include "JZNodeBind.h"
#include "JZWidgetBind.h"
#include "3rd/JZCommon/jzWidgets/JZLogWidget.h"

JZWidgetBind::JZWidgetBind()
{
}

JZWidgetBind::~JZWidgetBind()
{
}

void JZWidgetBind::bind()
{
    jzbind::ClassBind<JZLogWidget> cls_camera(Type_none, "JZLogWidget", "QWidget");    
    cls_camera.regist();
}

void JZWidgetBindInit()
{
    JZWidgetBind bind;
    bind.bind();
}