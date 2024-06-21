#ifndef JZNODE_RUSSIAN_H_
#define JZNODE_RUSSIAN_H_

#include "../SampleProject.h"

class SampleRussian : public SampleProject
{
public:
    SampleRussian();
    ~SampleRussian();

protected:    
    void addInitGame();
    void addInitFunction();
    void addMapGet();
    void addMapSet();    
    void addButtonClicked();
    void addCreateRect();
    void addRectDown();
    void addMoveFunction();
    void addGameLoop();
    void addPaintEvent();
    void addKeyEvent();
    void addRotate();
    void addCreateShape();
    void addCanPlaceShape();
    void addClearLine();
    QVector<QVector<QVector<QPoint>>> shapeGroup();
    
    int m_row = 20;
    int m_col = 10;
    int m_blockSize = 26;
};



#endif // !JZNODE_RUSSIAN_H_
