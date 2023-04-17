#include <QEventLoop>
#include <QDebug>
#include "JZProject.h"
#include "JZNodeCompiler.h"
#include "JZNodeEngine.h"
#include "JZNodeValue.h"

void testProjectSave()
{
    
}

void testBuild()
{
    JZNodeValue *v = new JZNodeValue();
    JZNodeAdd *add = new JZNodeAdd();

    JZProject project;
    int v_id = project.addNode(JZNodePtr(v));
    int add_id = project.addNode(JZNodePtr(add));
    project.addConnect(JZNodeGemo(v_id,0),JZNodeGemo(add_id,1));
    project.addConnect(JZNodeGemo(v_id,0),JZNodeGemo(add_id,2));

    JZNodeProgram program;

    qDebug().noquote() << program.dump();

    JZNodeCompiler compiler;
    if(compiler.build(&project,program))
    {
        QString text = program.dump();
        qDebug().noquote() << program.dump();        

        JZNodeEngine runner;
        runner.setProgram(&program);                        
        runner.start();
    }    
}
