#ifndef JZNODE_DEFINE_H_
#define JZNODE_DEFINE_H_

#include <QByteArray>

#if defined(_MSC_VER) && _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

constexpr int INVALID_ID = -1; 
QByteArray NodeMagic();

#endif