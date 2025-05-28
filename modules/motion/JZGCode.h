#ifndef JZ_GCODE_H_
#define JZ_GCODE_H_

#include "driver/MotionDriver.h"

class JZGCodeInstance
{
public:	
};

class JZGCode
{
public:	
	
};


//JZGCodeRunner
class JZGCodeRunner : public QObject
{
public:
	JZGCodeRunner(QObject *object);
	~JZGCodeRunner();
	
	void setDriver(MotionDriver *driver);
	void run();
	
protected:
	MotionDriver *m_driver;
};



#endif