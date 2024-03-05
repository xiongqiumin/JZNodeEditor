#include "JZNodeBuilder.h"
#include "JZParamItem.h"
#include "JZUiFile.h"
#include "JZClassItem.h"
#include "LogManager.h"

enum {
    build_global,
    build_emptyGlobal,
    build_unitTest,
};

JZNodeCustomBuild::JZNodeCustomBuild()
{
    m_type = Node_custom;
    addFlowIn();
    addFlowOut();
}

bool JZNodeCustomBuild::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeStart(m_id);
    if (!buildFunction(c, error))
        return false;

    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeBuilder
JZNodeBuilder::JZNodeBuilder(JZProject *project)
{
    m_project = project;
}

JZNodeBuilder::~JZNodeBuilder()
{

}

QString JZNodeBuilder::error() const
{
    return m_error;
}

void JZNodeBuilder::clear()
{
    m_connects.clear();
    m_error.clear();
    m_scripts.clear();
}

void JZNodeBuilder::initGlobal()
{            
    // init variable
    auto func = [this](JZNodeCompiler *c, QString&)->bool{
        auto global_params = m_project->globalVariableList();
        for(int i = 0; i < global_params.size(); i++)
        {
            auto def = m_project->globalVariable(global_params[i]);
            c->addAlloc(JZNodeIRAlloc::Heap, def->name, def->dataType(), def->value);
        }
        return true;
    };

    JZFunctionDefine global_func;
    global_func.name = "__init__";
    buildCustom(global_func, func);
}

bool JZNodeBuilder::build(JZNodeProgram *program)
{    
    clear();    
    m_program = program;        
    m_program->clear();    
    
    auto list = m_project->globalVariableList();
    for(int i = 0; i < list.size(); i++)
    {
        QString name = list[i];
        m_program->m_variables[name] = *m_project->globalVariable(name);
    }
    
    auto class_list = m_project->itemList("./",ProjectItem_class);
    for(int i = 0; i < class_list.size(); i++)
    {
        JZScriptClassItem *script = dynamic_cast<JZScriptClassItem*>(class_list[i]);        
        m_program->m_objectDefines << script->objectDefine();
                
        auto params = script->itemList(ProjectItem_param);
        Q_ASSERT(params.size() == 0 || params.size() == 1);
        if(params.size() == 1)
        {
            auto param = dynamic_cast<JZParamItem*>(params[0]);
            m_program->m_binds[script->className()] = param->bindVariables();
        }
    }

    auto bind_list = m_project->itemList("./", ProjectItem_scriptParamBinding);
    for(int i = 0; i < bind_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(bind_list[i]);
        if(!buildScriptFile(script))
            return false;
    }

    auto function_list = m_project->itemList("./", ProjectItem_scriptFunction);
    for (int i = 0; i < function_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(function_list[i]);
        if (!buildScriptFile(script))
            return false;        
    }

    auto flow_list = m_project->itemList("./", ProjectItem_scriptFlow);
    for(int i = 0; i < flow_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(flow_list[i]);
        if(!buildScriptFile(script))
            return false;
    }

    if(!link())
        return false;

    return true;
}

bool JZNodeBuilder::buildUnitTest(JZScriptItem *file, ScriptDepend *depend, JZNodeProgram *program)
{
    clear();    
    m_program = program;
    m_program->clear();

    if (!buildScriptFile(file))
        return false;

    if (file->getClassFile())
        m_program->m_objectDefines << file->getClassFile()->objectDefine();

    auto replaceStatment = [](JZNodeScript *script,int index)
    {
        auto ir_nop = new JZNodeIR(OP_nop);
        ir_nop->pc = script->statmentList[index]->pc;
        ir_nop->isBreak = script->statmentList[index]->isBreak;
        script->statmentList[index] = JZNodeIRPtr(ir_nop);
    };

    auto isDstVaild = [file](JZNodeIRParam *param, ScriptDepend *depend)->bool
    {
        if (!param->isRef())
            return true;

        auto type = JZNodeCompiler::variableCoor(file, param->ref());
        if (type == Variable_local)
            return true;

        return depend->indexOf(type == Variable_member, param->ref()) >= 0;
    };

    auto replaceParam = [file](JZNodeIRParam *param, ScriptDepend *depend)
    {
        if (!param->isRef())
            return;
        
        auto type = JZNodeCompiler::variableCoor(file, param->ref());
        if (type == Variable_local)
            return;

        auto def = depend->param(type == Variable_member, param->ref());
        *param = irLiteral(JZNodeType::initValue(def->dataType(), def->value));
    };

    auto script = m_scripts[file->itemPath()].data();
    for (int i = 0; i < script->statmentList.size(); i++)
    {
        auto *stmt = script->statmentList[i].data();
        switch (stmt->type)
        {
        case OP_set:
        {
            JZNodeIRSet *set = dynamic_cast<JZNodeIRSet*>(stmt);
            if (isDstVaild(&set->dst, depend))
                replaceParam(&set->src, depend);
            else
                replaceStatment(script,i);
            break;
        }
        case OP_add:
        case OP_sub:
        case OP_mul:
        case OP_div:
        case OP_mod:
        case OP_eq:
        case OP_ne:
        case OP_le:
        case OP_ge:
        case OP_lt:
        case OP_gt:
        case OP_and:
        case OP_or:
        case OP_bitand:
        case OP_bitor:
        case OP_bitxor:
        {
            JZNodeIRExpr *expr = dynamic_cast<JZNodeIRExpr*>(stmt);
            if (isDstVaild(&expr->dst, depend))
            {
                replaceParam(&expr->src1, depend);
                replaceParam(&expr->src2, depend);
            }
            else
            {   
                replaceStatment(script, i);                
            }      
            break;
        }
        default:
            break;
        }
    }

    auto it = depend->hook.begin();
    while (it != depend->hook.end())
    {
        auto rg_list = script->nodeInfo[it.key()].pcRanges;
        auto &param_list = it.value();
        auto &node_info = script->nodeInfo[it.key()];
        for (int rg_idx = 0; rg_idx < rg_list.size(); rg_idx++)
        {
            auto rg = rg_list[rg_idx];
            for (int pc = rg.start; pc < rg.end; pc++)
            {
                bool replace = true;
                auto *stmt = script->statmentList[pc].data();
                if (stmt->type == OP_set)
                {
                    JZNodeIRSet *set = dynamic_cast<JZNodeIRSet*>(stmt);
                    if (set->dst.isId())
                    {
                        int out_id = set->dst.id();
                        for (int out_idx = 0; out_idx < node_info.paramOut.size(); out_idx++)
                        {
                            if (node_info.paramOut[out_idx].id == out_id)
                            {
                                auto param = param_list[out_idx];
                                set->src = irLiteral(JZNodeType::initValue(param.dataType(), param.value));
                                replace = false;
                                break;
                            }
                        }                        
                    }
                }

                if(replace)
                    replaceStatment(script, pc);                
            }
        }
        it++;
    }

    JZFunctionDefine unit_func;
    unit_func.name = "__unittest__";           
    unit_func.paramOut = depend->function.paramOut;    

    auto build_unit = [this, depend](JZNodeCompiler *c, QString&)->bool {        
        int param_start = 0;
        if (depend->function.isMemberFunction())
        {            
            JZNodeIRParam irIn = irLiteral(depend->function.className);
            JZNodeIRParam irOut = irRef("__inst__");
            c->addCall("createObject", { irIn }, { irOut });
            param_start = 1;
        }
        for (int i = param_start; i < depend->function.paramIn.size(); i++)
        {
            auto &def = depend->function.paramIn[i];
            JZNodeIRAlloc *ir_alloc = new JZNodeIRAlloc();
            ir_alloc->type = JZNodeIRAlloc::StackId;
            ir_alloc->id = i;
            ir_alloc->dataType = def.dataType();
            ir_alloc->value = irLiteral(JZNodeType::defaultValue(def.dataType()));
            c->addStatement(JZNodeIRPtr(ir_alloc));          

            auto v = JZNodeType::initValue(def.dataType(), def.value);
            c->addSetVariable(irId(i), irLiteral(v));
        }
                                  
        for (int i = 0; i < depend->member.size(); i++)
        {
            auto &def = depend->member[i];
            auto ref = irRef("__inst__." + def.name);
            auto v = JZNodeType::initValue(def.dataType(),def.value);
            c->addSetVariable(ref, irLiteral(v));
        }

        if (depend->global.size() > 0)
        {
            for (int i = 0; i < depend->global.size(); i++)
            {   
                auto &def = depend->member[i];
                auto v = JZNodeType::initValue(def.dataType(), def.value);
                c->addAlloc(JZNodeIRAlloc::Heap, def.name, def.dataType(), irLiteral(v));
            }
        }               

        int node_id = c->currentNode()->id();

        QList<JZNodeIRParam> in, out;
        if (depend->function.isMemberFunction())
            in << irRef("__inst__");
        for (int i = param_start; i < depend->function.paramIn.size(); i++)
            in << irId(i);        
        for (int i = 0; i < depend->function.paramOut.size(); i++)
        {
            int out_id = c->currentNode()->paramOut(i);
            out << irId(c->paramId(node_id, out_id));
        }        

        c->addCall(depend->function.fullName(), in, out);
        return true;
    };
    
    m_program->m_scripts = m_scripts;

    QList<JZParamDefine> local_list;
    if (depend->function.isMemberFunction())
        local_list << JZParamDefine("__inst__", depend->function.className);
    if (!buildCustom(unit_func, build_unit, local_list))
        return false;

    JZFunctionDefine global_func;
    global_func.name = "__init__";
    if (!buildCustom(global_func, [](JZNodeCompiler*, QString&)->bool { return true; }))
        return false;
    
    return true;
}

bool JZNodeBuilder::buildCustom(JZFunctionDefine func, std::function<bool(JZNodeCompiler*, QString&)> buildFunction, const QList<JZParamDefine> &local)
{
    JZNodeScriptPtr boot = JZNodeScriptPtr(new JZNodeScript());    
    boot->file = func.name;

    JZScriptItem file(ProjectItem_scriptFunction);
    file.setFunction(func);
    file.setProject(m_project);
    for(int i = 0; i < local.size(); i++)
        file.addLocalVariable(local[i]);
    
    JZNodeCustomBuild *custom = new JZNodeCustomBuild();
    custom->buildFunction = buildFunction;
    file.addNode(JZNodePtr(custom));

    JZNodeReturn *ret = new JZNodeReturn();
    ret->setFunction(&func);

    file.addNode(JZNodePtr(ret));
    file.addConnect(custom->flowOutGemo(), ret->flowInGemo());
    for (int i = 0; i < func.paramOut.size(); i++)
    {
        auto id = custom->addParamOut("");
        custom->pin(id)->setDataType({ func.paramOut[i].dataType()});
        file.addConnect(custom->paramOutGemo(0), ret->paramInGemo(i));
    }

    auto start = file.getNode(0);
    file.addConnect(start->flowOutGemo(), custom->flowInGemo());

    JZNodeCompiler compiler;
    if (!compiler.build(&file, boot.data()))
    {
        m_error += compiler.error();
        LOGE(Log_Compiler, compiler.error());
        return false;
    }

    for (int i = 0; i < boot->statmentList.size(); i++)    
        boot->statmentList[i]->isBreak = false;    

    m_program->m_scripts[boot->file] = boot;
    return true;
}

bool JZNodeBuilder::buildScriptFile(JZScriptItem *scriptFile)
{       
    QString path = scriptFile->itemPath();
    if(!m_scripts.contains(path))    
        m_scripts[path] = JZNodeScriptPtr(new JZNodeScript());
    
    auto classFile = m_project->getItemClass(scriptFile);
    JZNodeScript *script = m_scripts[path].data();
    if(classFile)
        script->className = classFile->className();

    LOGI(Log_Compiler, "build " + scriptFile->itemPath());
    JZNodeCompiler compiler;
    if(!compiler.build(scriptFile,script))
    {        
        m_error += compiler.error();
        LOGE(Log_Compiler, compiler.error());
        return false;
    }
    
    return true;
}

bool JZNodeBuilder::link()
{       
    std::sort(m_program->m_objectDefines.begin(),m_program->m_objectDefines.end(),
        [](const JZNodeObjectDefine &d1,const JZNodeObjectDefine &d2)->bool{
            return d1.id < d2.id;
        });

    m_program->m_scripts = m_scripts;     

    JZFunctionDefine func;
    func.name = "__init__";    
    initGlobal();

    return true;
}
