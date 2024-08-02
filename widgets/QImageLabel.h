#ifndef QIMAGE_LABEL_H_
#define QIMAGE_LABEL_H_

#include <QWidget>

class QImageLabel : public QWidget
{
    Q_OBJECT

public:
    QImageLabel(QWidget *parent);
    ~QImageLabel();

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};


#endif // !QIMAGE_LABEL_H_
