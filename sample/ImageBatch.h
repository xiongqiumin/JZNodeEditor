#ifndef JZNODE_IAMGE_BATCH_H_
#define JZNODE_IAMGE_BATCH_H_

#include "SampleProject.h"

class SampleImageBatch : public SampleProject
{
public:
    SampleImageBatch();
    ~SampleImageBatch();

protected:    
    void addEvent();
    void addProcessImage();
    void addProcessDir();
    void addProcessGetImage();
};



#endif // !JZNODE_RUSSIAN_H_
