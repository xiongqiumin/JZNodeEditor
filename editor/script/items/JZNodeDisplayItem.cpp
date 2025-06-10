#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include "JZNodeDisplayItem.h"
#include "JZNodeValue.h"
#include "JZScriptItem.h"
#include "JZScriptItemVisitor.h"
#include "JZEditorGlobal.h"
#include "jzWidgets/JZImageLabel.h"
#include "jzWidgets/JZSelfLayout.h"
#include "modules/model/Yolo/JZYolo.h"
#include "modules/opencv/JZModuleOpencv.h"

using namespace cv;

//JZNodeDisplayItem
JZNodeDisplayItem::JZNodeDisplayItem(JZNode *node)
    :JZNodeGraphItem(node)
{
    QPushButton *btnAdd = new QPushButton("Add");
    btnAdd->connect(btnAdd, &QPushButton::clicked, [this] {
        this->onAddClicked();
    });
    
    m_addBlock = createWidgetBlock(btnAdd,true);
    m_addBlock->pri = 8;   
}

QString JZNodeDisplayItem::getInputType(int pin_id)
{
    JZScriptItemVisitor visitor(m_node->file());
    QList<JZNodePin*> input_pins = visitor.inputPin(m_id, pin_id);
    if (input_pins.size() != 1)
        return QString();

    auto pin = input_pins[0];
    auto env = editorEnvironment();
    int up_type = env->upType(env->nameListToTypeList(pin->dataType()));
    return env->typeToName(up_type);
}

void JZNodeDisplayItem::updatePin()
{
    JZNodeGraphItem::updatePin();
    for (auto b : m_blocks)
    {
        b->isShowName = false;

        QString type = getInputType(b->id);
        if (type == "Mat" || type == "QImage")
        {                      
            if (!b->widget || !b->widget->inherits("JZImageLabel"))
            {
                b->clearWidget();                                

                JZImageLabel *label = new JZImageLabel();

                QPushButton *btn = new QPushButton("...", label);
                btn->connect(btn, &QPushButton::clicked, [this, label]{
                    onLabelExpand(label);
                });

                btn->resize(32, 20);
                JZSelfLayout *layout = new JZSelfLayout(btn);
                layout->setWidget(btn);
                layout->setAlignment(Qt::AlignRight | Qt::AlignBottom);
                layout->setOffset(-2, -2);

                label->resize(160, 160);
                b->setWidget(label);
            }
        }
        else
        {
            if(b != m_addBlock)
                b->clearWidget();
        }
    }
}

void JZNodeDisplayItem::updateGraphics()
{

}

void JZNodeDisplayItem::clearValues()
{
    for (auto b : m_blocks)
        clearValue(b->id);
}

void JZNodeDisplayItem::clearValue(int pin)
{
    if (m_blocks[pin]->widget)
    {
        auto w = m_blocks[pin]->widget;
        if (w->inherits("JZImageLabel"))
        {
            JZImageLabel *label = qobject_cast<JZImageLabel*>(w);
            label->setImage(QImage());
        }
    }
    else
    {
        setPinRuntimeValue(pin, QString());
        update();
    }
}

void JZNodeDisplayItem::setValue(int pin,QVariantPtr *ref)
{    
    QString type_name = editorEnvironment()->typeToName(ref->type);
    if (type_name == "Mat" || type_name == "QImage")
    {
        JZImageLabel *label = qobject_cast<JZImageLabel*>(m_blocks[pin]->widget);
        
        if (type_name == "Mat")
        {
            Mat *mat = JZObjectCast<Mat>(toJZObject(*ref->ptr));
            QImage image = QtOcv::mat2Image(*mat);
            label->setImage(image);
        }
        else
        {
            QImage *image = JZObjectCast<QImage>(toJZObject(*ref->ptr));            
            label->setImage(*image);
        }
    }
    else if (type_name == "QList<JZYoloResult>")
    {
        QList<JZYoloResult> *yolo_ret = JZObjectCast<QList<JZYoloResult>>(toJZObject(*ref->ptr));
        QList<JZGraphic> graphList = JZYoloResult::toGraphics(*yolo_ret);        

        QList<int> in_list = blockList(true);
        int index = in_list.indexOf(pin);
        for(int i = index - 1; i >= 0; i--)
        {
            int pin_id = in_list[i];
            if(m_blocks[pin_id]->widget && m_blocks[pin_id]->widget->inherits("JZImageLabel"))
            {
                JZImageLabel *label = qobject_cast<JZImageLabel*>(m_blocks[pin_id]->widget);
                label->setGraphics(graphList);
            }
        }
    }
    
    setPinRuntimeValue(pin, editorEnvironment()->debugString(*ref->ptr));
    update();
}

void JZNodeDisplayItem::onAddClicked()
{
    JZNodeDisplay *node_display = (JZNodeDisplay *)m_node;

    QByteArray oldValue = saveNode();
    node_display->addInput();
    notifyPropChanged(oldValue);
}

void JZNodeDisplayItem::onLabelExpand(JZImageLabel *label)
{
    QScrollArea *area = new QScrollArea();
    area->setAttribute(Qt::WA_DeleteOnClose, true);

    QImage image = label->image();

    JZImageLabel *w = new JZImageLabel();
    w->setImage(image);    
    if (!image.isNull())
        w->setFixedSize(image.width(), image.height());

    w->setGraphics(label->graphics());
    area->setWidget(w);
    area->resize(600, 400);
    area->show();
}