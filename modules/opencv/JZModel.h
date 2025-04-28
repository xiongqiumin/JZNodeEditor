#ifndef JZ_MODEL_H_
#define JZ_MODEL_H_

#include <QString>

class JZModel
{
public:
	JZModel();
	virtual ~JZModel();

	virtual bool loadNet(QString path) = 0;
};

#endif