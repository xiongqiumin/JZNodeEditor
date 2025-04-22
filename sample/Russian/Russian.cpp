#include <QApplication>
#include <QDir>
#include "Russian.h"
#include "JZNodeBuilder.h"
#include "JZNodeVM.h"
#include "UiCommon.h"
#include "JZNodeFunction.h"
#include "JZNodeValue.h"
#include "JZUiItem.h"
#include "JZNodeView.h"
#include "JZNodeUtils.h"
#include "JZContainer.h"

SampleRussian::SampleRussian()
{    
    QFileInfo info(__FILE__);
    m_root = info.path();
    
    newProject("russian");        
    
    auto class_file = m_project.getClass("MainWindow");

    JZUiItem* ui_file = class_file->ui();
    ui_file->setXml(loadUi("Russian.ui"));
    m_project.saveItem(ui_file);
}

SampleRussian::~SampleRussian()
{

}

/*

class Russian : public QMainWindow
{

public:
    Russian(QWidget *parent = nullptr);
    ~Russian();

protected slots:
    void onTimerOut();
    void on_btnStart_clicked();
    void on_btnStop_clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void initGame();
    void startGame();
    void stopGame();

    void setMap(int i, int j, int value);
    int getMap(int i, int j);
    void gameLoop();

    bool canPlace(int x,int y,int type);
    void place();

    bool canRotate();   
    bool canMoveDown();           
    bool canMoveLeft();
    bool canMoveRight();

    void rotate();
    void moveDown();
    void moveLeft();
    void moveRight();

    QList<QList<QList<QPoint>>> shapeGroup();
    void genShape();
    QList<QList<int>> m_map;
    QList<QColor> m_colorList;
    QList<QList<QList<QPoint>>> m_shapeItems;

    int m_row;
    int m_col;
    int m_shapeRow;
    int m_shapeCol;
    int m_rotate;
    int m_shapeIndex;
    bool m_isRect;
    QTimer* m_timer;
    int BLOCK_SIZE;

    Ui_RussianClass ui;
};

Russian::Russian(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    BLOCK_SIZE = 22;
    initGame();
}

Russian::~Russian() 
{
}

void Russian::onTimerOut()
{
    gameLoop();
}

void Russian::on_btnStart_clicked()
{
    startGame();
}
void Russian::on_btnStop_clicked()
{
    stopGame();
}

QList<QList<QList<QPoint>>> Russian::shapeGroup()
{
    QList<QList<QList<QPoint>>> shape_group;

    QList<QPoint> pt_box;
    pt_box << QPoint(0, 0) << QPoint(0, 1) << QPoint(1, 0) << QPoint(1, 1);

    QList<QList<QPoint>> box_group;
    box_group << pt_box;
    shape_group << box_group;

    QList<QPoint> pt_line1, pt_line2;
    pt_line1 << QPoint(0, 0) << QPoint(1, 0) << QPoint(2, 0) << QPoint(3, 0);
    pt_line2 << QPoint(1, -2) << QPoint(1, -1) << QPoint(1, 0) << QPoint(1, 1);

    QList<QList<QPoint>> line_group;
    line_group << pt_line1 << pt_line2;
    shape_group << line_group;

    QList<QPoint> pt_tri1, pt_tri2, pt_tri3, pt_tri4;
    pt_tri1 << QPoint(0, 1) << QPoint(1, 0) << QPoint(1, 1) << QPoint(1, 2);
    pt_tri2 << QPoint(0, 1) << QPoint(1, 1) << QPoint(2, 1) << QPoint(1, 2);
    pt_tri3 << QPoint(1, 0) << QPoint(1, 1) << QPoint(1, 2) << QPoint(2, 1);
    pt_tri4 << QPoint(1, 0) << QPoint(0, 1) << QPoint(1, 1) << QPoint(2, 1);

    QList<QList<QPoint>> tri_group;
    tri_group << pt_tri1 << pt_tri2 << pt_tri3 << pt_tri4;
    shape_group << tri_group;

    QList<QPoint> pt_left1, pt_left2, pt_left3, pt_left4;
    pt_left1 << QPoint(0, 0) << QPoint(0, 1) << QPoint(0, 2) << QPoint(1, 0);
    pt_left2 << QPoint(0, 1) << QPoint(0, 2) << QPoint(1, 2) << QPoint(2, 2);
    pt_left3 << QPoint(1, 0) << QPoint(1, 1) << QPoint(1, 2) << QPoint(0, 2);
    pt_left4 << QPoint(0, 0) << QPoint(1, 0) << QPoint(2, 0) << QPoint(2, 1);

    QList<QList<QPoint>> left_group;
    left_group << pt_left1 << pt_left2 << pt_left3 << pt_left4;
    shape_group << left_group;

    QList<QPoint> pt_right1, pt_right2, pt_right3, pt_right4;
    pt_right1 << QPoint(0, 0) << QPoint(0, 1) << QPoint(0, 2) << QPoint(1, 2);
    pt_right2 << QPoint(2, 1) << QPoint(0, 2) << QPoint(1, 2) << QPoint(2, 2);
    pt_right3 << QPoint(1, 0) << QPoint(2, 0) << QPoint(2, 1) << QPoint(2, 2);
    pt_right4 << QPoint(0, 0) << QPoint(1, 0) << QPoint(2, 0) << QPoint(0, 1);

    QList<QList<QPoint>> right_group;
    right_group << pt_right1 << pt_right2 << pt_right3 << pt_right4;
    shape_group << right_group;

    return shape_group;
}

void Russian::paintEvent(QPaintEvent* event) 
{
    QPainter pt(this);

    int x_offset = 10;
    int y_offset = 10;
    pt.fillRect(x_offset, y_offset, BLOCK_SIZE * m_col, BLOCK_SIZE * m_row, Qt::white);
    pt.drawRect(x_offset, y_offset, BLOCK_SIZE * m_col, BLOCK_SIZE * m_row);
    for (int y = 0; y < m_row; y++)
    {
        for (int x = 0; x < m_col; x++)
        {
            if (m_map[y][x] == 1)
                pt.fillRect(QRect(x_offset + x * BLOCK_SIZE, y_offset + y * BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE), Qt::black);
        }
    }

    if (m_isRect)
    {
        auto shape = m_shapeItems[m_shapeIndex][m_rotate];
        for (int shape_idx = 0; shape_idx < shape.size(); shape_idx++)
        {
            auto s = shape[shape_idx];
            int x = m_shapeCol + s.x();
            int y = m_shapeRow + s.y();
            pt.fillRect(QRect(x_offset + x * BLOCK_SIZE, y_offset + y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE), m_colorList[m_shapeIndex]);
        }
    }
}

void Russian::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Up)
    {
        if (canRotate())
            rotate();
    }
    else if (event->key() == Qt::Key_Down)
    {
        if (canMoveDown())
            moveDown();
    }
    else if (event->key() == Qt::Key_Left)
    {
        if (canMoveLeft())
            moveLeft();
    }
    else if (event->key() == Qt::Key_Right)
    {
        if (canMoveRight())
            moveRight();
    }
    update();
}

void Russian::initGame()
{
    m_shapeItems = shapeGroup();
    m_colorList = { Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::cyan };
    m_row = 20;
    m_col = 12;
    m_isRect = false;

    for (int i = 0; i < m_row; i++)
    {
        QList<int> line;
        line.resize(m_col);
        m_map.push_back(line);
    }
    for (int y = 0; y < m_row; y++)
    {
        for (int x = 0; x < m_col; x++)
        {
            m_map[y][x] = 0;
        }
    }

    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, &Russian::onTimerOut);
}

void Russian::startGame()
{
    for (int y = 0; y < m_row; y++)
    {
        for (int x = 0; x < m_col; x++)
        {
            m_map[y][x] = 0;
        }
    }

    genShape();
    m_timer->start(50);
    update();
}

void Russian::stopGame()
{
    update();
}

void Russian::gameLoop()
{
    if (m_isRect)
    {
        if (canMoveDown())
        {
            moveDown();
        }
        else
        {
            place();
            m_isRect = false;
        }
    }
    else
    {
        genShape();
        if (!canPlace(m_shapeRow, m_shapeCol, m_rotate))
        {
            m_timer->stop();
        }
    }
    update();
}

void Russian::setMap(int i, int j, int value)
{
    m_map[i][j] = value;
}

int Russian::getMap(int i, int j)
{
    return m_map[i][j];
}

bool Russian::canPlace(int shape_row, int shape_col, int rotate)
{
    auto shape = m_shapeItems[m_shapeIndex][rotate];
    for (int shape_idx = 0; shape_idx < shape.size(); shape_idx++)
    {
        auto s = shape[shape_idx];
        int x = shape_col + s.x();
        int y = shape_row + s.y();
        if (x < 0 || x >= m_col)
            return false;
        if (y < 0 || y >= m_row)
            return false;

        if (m_map[y][x] != 0)
            return false;
    }

    return true;
}

void Russian::place()
{
    auto shape = m_shapeItems[m_shapeIndex][m_rotate];
    for (int shape_idx = 0; shape_idx < shape.size(); shape_idx++)
    {
        auto s = shape[shape_idx];
        int row = m_shapeRow + s.y();
        int col = m_shapeCol + s.x();
        m_map[row][col] = 1;
    }
    m_isRect = false;
}

bool Russian::canRotate()
{
    return canPlace(m_shapeRow, m_shapeCol, (m_rotate + 1) % m_shapeItems[m_shapeIndex].size());
}

bool Russian::canMoveDown() 
{
    return canPlace(m_shapeRow+1, m_shapeCol, m_rotate);
}

bool Russian::canMoveLeft()
{
    return canPlace(m_shapeRow, m_shapeCol -1, m_rotate);
}

bool Russian::canMoveRight()
{
    return canPlace(m_shapeRow,  m_shapeCol+ 1, m_rotate);
}

void Russian::genShape()
{
    m_shapeRow = 2;
    m_shapeCol = rand() % (m_col - 4) + 2;

    m_shapeIndex = rand() % m_shapeItems.size();
    m_rotate = rand() % m_shapeItems[m_shapeIndex].size();
    m_isRect = true;
}

void Russian::rotate()
{
    m_rotate = (m_rotate + 1)% m_shapeItems[m_shapeIndex].size();
}

void Russian::moveDown()
{
    m_shapeRow++;
}

void Russian::moveLeft()
{
    m_shapeCol--;
}

void Russian::moveRight()
{
    m_shapeCol++;
}
*/