#ifndef UI_COMMON_H_
#define UI_COMMON_H_

#if defined(_MSC_VER) && _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMessageBox>

class QTreeWidget;
class UiHelper
{
public:
    static void filter(QTreeWidget *tree, QString name);
};


#endif