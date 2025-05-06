#include <QComboBox>
#include <QPushButton>
#include "JZNodeDisplayItem.h"
#include "JZNodeValue.h"
#include "JZScriptItem.h"
#include "JZScriptItemVisitor.h"
#include "JZEditorGlobal.h"
#include "jzWidgets/JZImageLabel.h"
#include "modules/opencv/JZModuleOpencv.h"

//JZNodeDisplayItem
JZNodeDisplayItem::JZNodeDisplayItem()
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

                QWidget *widget = new JZImageLabel();
                widget->resize(160, 160);
                b->setWidget(widget);
            }
        }
        else
        {
            if(b != m_addBlock)
                b->clearWidget();
        }
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

    }
    else
    {

    }
    update();
}

void JZNodeDisplayItem::onAddClicked()
{
    JZNodeDisplay *node_display = (JZNodeDisplay *)m_node;

    QByteArray oldValue = saveNode();
    node_display->addInput();
    notifyPropChanged(oldValue);
}