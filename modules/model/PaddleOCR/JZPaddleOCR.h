#ifndef JZ_PADDLE_OCR_H_
#define JZ_PADDLE_OCR_H_

#include <QRect>
#include <QString>
#include <QObject>
#include "ocr_det.h"
#include "ocr_cls.h"
#include "ocr_rec.h"
#include "jzWidgets/JZImageGraphic.h"

class JZOCRResult
{
public:
    static QList<JZGraphic> toGraphics(const QList<JZOCRResult> &result);

    JZOCRResult();    

    QRect rect;
    QString text;
    double score;
};

class JZPaddleOCR : public QObject
{
    Q_OBJECT

public:
    JZPaddleOCR();
    ~JZPaddleOCR();
    
    bool isInit();
    bool init();
    void deinit();
    QList<JZOCRResult> ocr(cv::Mat mat);

protected:    
    bool m_init;
    PaddleOCR::DBDetector m_detector;
    PaddleOCR::Classifier m_cls;
    PaddleOCR::CRNNRecognizer m_rec;
};


#endif