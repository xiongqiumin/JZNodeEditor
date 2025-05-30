#ifndef JZ_VISION_ITEM_BASE_H_
#define JZ_VISION_ITEM_BASE_H_

#include <QGraphicsItem>

enum 
{
    VisionItem_none = QGraphicsItem::UserType,
	Vision_itemLine, 
	Vision_itemNode,
};

class JZVisionView;
class JZVisionItemBase : public QGraphicsItem
{
public:
    JZVisionItemBase();
    ~JZVisionItemBase();

    int id() const;
    void setId(int id);

    virtual void updateNode() = 0;
    virtual int type() const override;
    JZVisionView* editor() const;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    int m_id;
    int m_type;
};

#endif // !JZ_VISION_ITEM_BASE_H_
