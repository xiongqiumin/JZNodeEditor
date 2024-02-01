#ifndef JZNODE_UTILS_H_
#define JZNODE_UTILS_H_

#include <QString>
#include "JZProject.h"

QString makeLink(QString filename, QString function, int nodeId);
void projectUpdateLayout(JZProject *project);


#endif // !JZNODE_UTILS_H_