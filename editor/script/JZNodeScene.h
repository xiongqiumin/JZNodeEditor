
#ifndef JZNODE_SCENE_H_
#define JZNODE_SCENE_H_

#include <QGraphicsScene>

class JZNodeScene : public QGraphicsScene
{
    Q_OBJECT

public:
    JZNodeScene();
    ~JZNodeScene();

protected:    
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
};

#endif
