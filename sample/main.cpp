#include <QApplication>
#include "JZNodeInit.h"
#include "Russian/Russian.h"
#include "SmartHome/SmartHome.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JZNodeInit();   
    
    {
        SampleRussian russian;
        russian.saveProject();
    }

    {
        SampleSmartHome smarthome;
        smarthome.saveProject();
    }

    return 0;
}