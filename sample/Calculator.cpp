#include "Calculator.h"
#include "JZUiFile.h"

SampleCalculator::SampleCalculator()
{
    m_name = "calculator";
    m_project.initUi();
    
    JZUiFile *ui_file = dynamic_cast<JZUiFile*>(m_project.getItem("./mainwindow/mainwindow.ui"));
    ui_file->setXml(loadUi("Calculator.ui"));    
}

SampleCalculator::~SampleCalculator()
{

}