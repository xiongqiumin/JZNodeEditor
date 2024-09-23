#ifndef JZNODE_CORE_API_H_
#define JZNODE_CORE_API_H_

#include <QString>

#ifdef JZCORE_SDK_EXPORTS
#define JZCORE_EXPORT Q_DECL_EXPORT
#else
#define JZCORE_EXPORT Q_DECL_IMPORT
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#endif