#ifndef JZ_VISION_CORUTINE_H_
#define JZ_VISION_CORUTINE_H_

#include "JZEngineCoroutine.h"

class JZVisionCoroutine : public JZEngineCoroutine
{
public:
	JZVisionCoroutine(JZNodeEngine* engine);

	QString function;
};


#endif // ! JZ_VISION_CORUTINE_H_
