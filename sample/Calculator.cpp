#include "Calculator.h"
#include "JZUiFile.h"

SampleCalculator::SampleCalculator()
{
    newProject("calcualtor");
        
    JZUiFile *ui_file = dynamic_cast<JZUiFile*>(m_project.getItem("./mainwindow.ui"));
    ui_file->setXml(loadUi("Calculator.ui"));
    m_project.saveItem(ui_file);
}

SampleCalculator::~SampleCalculator()
{

}