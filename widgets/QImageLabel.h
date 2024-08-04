#ifndef QIMAGE_LABEL_H_
#define QIMAGE_LABEL_H_

#include <QWidget>

class QImageLabel : public QWidget
{
    Q_OBJECT

public:
    QImageLabel(QWidget *parent = nullptr);
    ~QImageLabel();

    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;    
};


#endif // !QIMAGE_LABEL_H_
