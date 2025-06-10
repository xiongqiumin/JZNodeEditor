#ifndef JZ_PADDLE_OCR_H_
#define JZ_PADDLE_OCR_H_

#include <QRect>
#include <QString>
#include <QObject>
#include "ocr_det.h"
#include "ocr_cls.h"
#include "ocr_rec.h"

class JZOCRResult
{
public:
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
    void init();
    QList<JZOCRResult> ocr(cv::Mat mat);

protected:    
    bool m_init;
    PaddleOCR::DBDetector m_detector;
    PaddleOCR::Classifier m_cls;
    PaddleOCR::CRNNRecognizer m_rec;
};


#endif