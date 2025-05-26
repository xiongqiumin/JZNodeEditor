#ifndef JZTITLEWIDGET_H
#define JZTITLEWIDGET_H

#include <QWidget>
#include <QPixmap>

class JZTitleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JZTitleWidget(QWidget *parent = nullptr);
    ~JZTitleWidget();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_image;
};

#endif // JZTITLEWIDGET_H    