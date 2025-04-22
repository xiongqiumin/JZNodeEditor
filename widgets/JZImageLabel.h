#ifndef QIMAGE_LABEL_H_
#define QIMAGE_LABEL_H_

#include <QWidget>

class JZImageLabel : public QWidget
{
    Q_OBJECT

public:
    JZImageLabel(QWidget *parent = nullptr);
    ~JZImageLabel();

    QImage image();
    void setImage(QImage image);

    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;    

    QImage m_image;
};


#endif // !QIMAGE_LABEL_H_
