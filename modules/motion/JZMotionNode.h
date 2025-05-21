#ifndef JZ_MOTION_NODE_H_
#define JZ_MOTION_NODE_H_

#include "JZNode.h"
#include "../JZModuleDefine.h"
#include "JZMotion.h"

enum MotionNode
{
    Node_MotionInit = Module_MotionNode,
    Node_MotionZero,
    Node_MotionMove
};

class JZNodeMotionZero : public JZNode
{
public:
    JZNodeMotionZero();
    ~JZNodeMotionZero();

    bool compiler(JZNodeCompiler *c, QString &error);

protected:
   
};

class JZNodeMotionInit : public JZNode
{
public:
    JZNodeMotionInit();
    ~JZNodeMotionInit();

    bool compiler(JZNodeCompiler *c, QString &error);

protected:

};

class JZNodeMotionMove : public JZNode
{
public:
    JZNodeMotionMove();
    ~JZNodeMotionMove();    

    bool compiler(JZNodeCompiler *c, QString &error);
    
protected:

};

#endif // ! JZ_MOTION_NODE_H_
