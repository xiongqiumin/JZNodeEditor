#ifndef JZ_YOLO_VIEW_H_
#define JZ_YOLO_VIEW_H_

#include "3rd/JZCommon/jzWidgets/JZImageView.h"


class JZYoloView : public JZImageView
{
    Q_OBJECT
    
public:
    JZYoloView();
    ~JZYoloView();

    void setYoloResult(QImage image,const QList<YoloResult> &m_lists);
    QList<QColor> m_colorList;
};


#endif