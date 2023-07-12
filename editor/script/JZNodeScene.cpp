#include "JZNodeScene.h"
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>

JZNodeScene::JZNodeScene()
{
}

JZNodeScene::~JZNodeScene()
{
}

void JZNodeScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void JZNodeScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void JZNodeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

