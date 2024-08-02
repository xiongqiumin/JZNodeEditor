#include <QVBoxLayout>
#include <QTextEdit>
#include "JZAboutDialog.h"

JZAboutDialog::JZAboutDialog(QWidget *parent)
    :JZBaseDialog(parent)
{
    QVBoxLayout *v = new QVBoxLayout();
    v->setContentsMargins(0, 0, 0, 0);

    QTextEdit *edit = new QTextEdit();
    edit->setReadOnly(true);
    v->addWidget(edit);

    m_mainWidget->setLayout(v);

    QString tips = R"(我们是开发基于qt的低代码平台，类似蓝图的节点编译器，支持qt设计器布局，控件参数绑定，流程代码，自定义插件，调试功能，并可以导出c++程序。
本产品可以结合视觉算法，采集，控制等功能，快速开发出属于贵司自己的低代码平台，大幅降低开发，部署和调试的时间。
详细咨询可添加QQ群

主要特点
1.基于qt开发，单机应用，简单高效.
2.完整的程序功能，强类型，支持分支，循环，判断，信号，槽，泛型，类，lambda，实时预览，可以构建复杂应用。
3.方便扩展，提供类似pybind接口，快速绑定c++.
4.除节点编辑器外，提供等价脚本支持，也就是同时支持连线开发和脚本开发，可以互相调用。
5.aot支持，可以导出为c++代码.
6.编辑器扩展，用户可以方便的扩展场景，构建属于自己的低代码应用。

预期使用场景
用户基于我们的低代码平台进行二次开发，通过增加库，或更多的场景编辑。
1.工业自动化，机器视觉
1.智能家居，类似node - red
2.上位机应用，类似labview
3.工业网关
4.图像处理，类似blender
5.单片机，plc开发
6.ai应用
因为产品本身是低代码平台，理论应用场景是很广阔的。)";

    QString text = QString("欢迎试用\n版本号:") + __DATE__ " " __TIME__;
    text += "\n文件->打开->项目 可以打开 sample 下面的一个 demo 工程.";
    text += "\nQQ群:598601341";
    text += "\n\n" + tips;
    edit->setText(text);

    showButton(Button_Cancel,false);
    resize(400, 500);
}

bool JZAboutDialog::onOk()
{
    return true;
}