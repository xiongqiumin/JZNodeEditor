#pragma once

#include "JZModel.h"
#include <QString>
#include <QImage>
#include <vector>

class JZModelOCRConfig : public JZModelConfig
{
public:
    JZModelOCRConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    JZModelBackEnd backend;
    QString modelPath;
};
QDataStream& operator<<(QDataStream& s, const JZModelOCRConfig& param);
QDataStream& operator>>(QDataStream& s, JZModelOCRConfig& param);

// OCR识别结果结构
struct JZOCRResult {
    QString text;          // 识别文本
    float confidence;      // 置信度
    QRect boundingBox;     // 文本框位置
};

// OCR模型接口
class JZModelOCR : public JZModel 
{
public:
    QString name() const;
    const JZModelConfigEnum &config();
	void setConfig(JZModelConfigEnum config);

    virtual bool isInit() override;
	virtual bool init() override;
	virtual void deinit() override;
	
	QList<JZOCRResult> forward(Mat mat);
	
protected:	
	JZModelEnginePtr m_net;
};