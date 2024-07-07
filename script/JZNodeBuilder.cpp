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

//JZNodeCustomBuild
JZNodeCustomBuild::JZNodeCustomBuild()
{
    m_type = Node_custom;
    addFlowIn();
    addFlowOut();
}

bool JZNodeCustomBuild::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeDebug(m_id);
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
            c->addAlloc(JZNodeIRAlloc::Heap, def->name, def->dataType());
            if(!def->value.isEmpty())
            {
                if(!JZNodeType::isObject(def->dataType()))
                    c->addSetVariable(irRef(def->name),irLiteral(JZNodeType::initValue(def->dataType(),def->value)));
                else
                {
                    QString function = def->type + ".__fromString__";
                    QList<JZNodeIRParam> in,out;
                    in << irLiteral(def->value);
                    out << irRef(def->name);
                    c->addCall(function,in,out);
                }
            }
        }
        return true;
    };

    JZFunctionDefine global_func;
    global_func.name = "__init__";
    global_func.isFlowFunction = true;
    buildCustom(global_func, func);
}

CompilerInfo JZNodeBuilder::compilerInfo(JZScriptItem *file) const
{
    auto it = m_scripts.find(file->itemPath());
    if (it == m_scripts.end())
        return CompilerInfo();

    return it->compilerInfo;
}

bool JZNodeBuilder::buildScript(JZScriptItem *scriptFile)
{
    QString path = scriptFile->itemPath();
    m_scripts[path].script = JZNodeScriptPtr(new JZNodeScript());

    auto classFile = m_project->getItemClass(scriptFile);
    JZNodeScript *script = m_scripts[path].script.data();
    script->clear();
    if (classFile)
        script->className = classFile->className();

    LOGI(Log_Compiler, "build " + scriptFile->itemPath());
    JZNodeCompiler compiler;
    bool ret = compiler.build(scriptFile, script);
    m_scripts[path].compilerInfo = compiler.compilerInfo();
    if(!ret)
    {
        m_error += compiler.error();
        LOGE(Log_Compiler, compiler.error());
        return false;
    }

    return true;
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
    
    JZNodeTypeMeta type_meta;
    auto class_list = m_project->itemList("./",ProjectItem_class);
    for(int i = 0; i < class_list.size(); i++)
    {
        JZScriptClassItem *script = dynamic_cast<JZScriptClassItem*>(class_list[i]);        
        type_meta.objectList << script->objectDefine();
                
        auto params = script->itemList(ProjectItem_param);
        Q_ASSERT(params.size() == 0 || params.size() == 1);
        if(params.size() == 1)
        {
            auto param = dynamic_cast<JZParamItem*>(params[0]);
            m_program->m_binds[script->className()] = param->bindVariables();
        }
    }

    auto container_list = m_project->containerList();
    for(int i = 0; i < container_list.size(); i++)
    {
        auto meta = JZNodeObjectManager::instance()->meta(container_list[i]);
        JZNodeCObjectDelcare cobj;
        cobj.className = meta->className;
        cobj.id = meta->id;
        type_meta.cobjectList << cobj;
    }

    auto bind_list = m_project->itemList("./", ProjectItem_scriptParamBinding);
    for(int i = 0; i < bind_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(bind_list[i]);
        if(!buildScript(script))
            return false;
    }

    auto function_list = m_project->itemList("./", ProjectItem_scriptFunction);
    for (int i = 0; i < function_list.size(); i++)
    {
        JZScriptItem *script = dynamic_cast<JZScriptItem*>(function_list[i]);
        if (!buildScript(script))
            return false;

        auto func_def = script->function();
        if(!func_def.isMemberFunction())
            type_meta.functionList << func_def;        
    }

    m_program->m_typeMeta = type_meta;
    if(!link())
        return false;

    return true;
}

void JZNodeBuilder::replaceNopStatment(JZNodeScript *script, int index)
{
    auto ir_nop = new JZNodeIR(OP_nop);
    ir_nop->pc = script->statmentList[index]->pc;
    script->statmentList[index] = JZNodeIRPtr(ir_nop);
}

void JZNodeBuilder::replaceUnitTestParam(JZScriptItem *file,ScriptDepend *depend)
{
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

    auto script = m_scripts[file->itemPath()].script.data();
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
                replaceNopStatment(script, i);
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
                replaceNopStatment(script, i);
            }
            break;
        }
        default:
            break;
        }
    }
}

void JZNodeBuilder::replaceUnitTestFunction(JZScriptItem *file, ScriptDepend *depend)
{
    auto script = m_scripts[file->itemPath()].script.data();
/*
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

                if (replace)
                    replaceNopStatment(script, pc);
            }
        }
        it++;
    }
*/
}

bool JZNodeBuilder::buildUnitTest(JZScriptItem *file, ScriptDepend *depend, JZNodeProgram *program)
{
    build(program);
    m_scripts.remove(file->itemPath());
    if (!buildScript(file))
        return false;        

    auto script = m_scripts[file->itemPath()].script.data();
    JZNodeScriptPtr tmp_script = JZNodeScriptPtr(script->clone());
    
    replaceUnitTestParam(file, depend);
    replaceUnitTestFunction(file, depend);
    
    JZFunctionDefine unit_func;
    unit_func.name = depend->function.fullName() + "__unittest__";
    for (int i = 0; i < depend->function.paramIn.size(); i++)
        unit_func.paramIn.push_back(JZParamDefine("in" + QString::number(i), Type_string));
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
            //ir_alloc->value = irLiteral(JZNodeType::defaultValue(def.dataType()));
            c->addStatement(JZNodeIRPtr(ir_alloc));          
            //c->addSetVariable(irId(i), irId(Reg_Call + i));
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
                //c->addAlloc(JZNodeIRAlloc::Heap, def.name, def.dataType(), irLiteral(v));
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

    QList<JZParamDefine> local_list;
    if (depend->function.isMemberFunction())
        local_list << JZParamDefine("__inst__", depend->function.className);
    if (!buildCustom(unit_func, build_unit, local_list))
        return false;

    link();    
    return true;
}

bool JZNodeBuilder::buildCustom(JZFunctionDefine func, std::function<bool(JZNodeCompiler*, QString&)> buildFunction, const QList<JZParamDefine> &local)
{
    JZNodeScriptPtr boot = JZNodeScriptPtr(new JZNodeScript());    

    JZScriptItem file(ProjectItem_scriptFunction);
    file.setName(func.name);
    file.setFunction(func);
    file.setProject(m_project);
    for(int i = 0; i < local.size(); i++)
        file.addLocalVariable(local[i]);
    
    JZNodeCustomBuild *custom = new JZNodeCustomBuild();
    custom->buildFunction = buildFunction;
    file.addNode(custom);

    JZNodeReturn *ret = new JZNodeReturn();
    file.addNode(ret);
    
    file.addConnect(custom->flowOutGemo(), ret->flowInGemo());
    for (int i = 0; i < func.paramOut.size(); i++)
    {
        auto id = custom->addParamOut("");
        custom->pin(id)->setDataType({ func.paramOut[i].type });
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

    m_program->m_scripts[boot->file] = boot;
    m_program->m_typeMeta.functionList << file.function();
    return true;
}

bool JZNodeBuilder::link()
{       
    auto it_s = m_scripts.begin();
    while (it_s != m_scripts.end())
    {
        m_program->m_scripts[it_s.key()] = it_s->script;
        it_s++;
    }

    JZFunctionDefine func;
    func.name = "__init__";    
    initGlobal();

    return true;
}
