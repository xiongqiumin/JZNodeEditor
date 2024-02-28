#include "Calculator.h"
#include "JZUiFile.h"

SampleCalculator::SampleCalculator()
{
    newProject("calcualtor");
    
    JZUiFile *ui_file = dynamic_cast<JZUiFile*>(m_project.getItem("./mainwindow/mainwindow.ui"));
    ui_file->setXml(loadUi("Calculator.ui"));    
}

SampleCalculator::~SampleCalculator()
{

}