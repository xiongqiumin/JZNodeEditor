#include <QTest>
#include <math.h>
#include <functional>
#include "test_benchmark.h"
#include "JZContainer.h"
#include "JZNodeFunction.h"
#include "JZNodeValue.h"

#define JZBENCHMARK(name) m_benchmark.reset(#name);  \
    while(m_benchmark.run()) \
    for(int bench_idx = 0; bench_idx < m_benchmark.step(); bench_idx++)

/*
todo:
执行的时候可以考虑把 CFunction 改成 call(QList<QVarinat*> in,QList<QVarinat*> out)
call 的时候 checkFucntion 和拷贝数据到 reg 比较耗时
*/
BenchmarkTest::Benchmark::Benchmark()
{
    m_count = 0;
}

void BenchmarkTest::Benchmark::reset(QString name)
{
    m_name = name;
    m_first = true;
    m_count = 0;
    m_step = 1;
    m_stepStart = 0;
    m_timer.restart();
}

void BenchmarkTest::Benchmark::clear()
{
    reset("");
    m_runInfo.clear();
}

void BenchmarkTest::Benchmark::report()
{
    QString text = "\nReport:\n";
    for(int i = 0; i < m_runInfo.size(); i++)
    {
        auto &info = m_runInfo[i];
        double cost = (double)info.time / info.count / 1000000.0;
        text += info.name + "(" + QString::number(cost,'f')  + " ms, ";
        text += "step " + QString::number(m_step) + ", ";
        text += "count " + QString::number(info.count) + ")\n";
    }
    qDebug().noquote() << text;
}

bool BenchmarkTest::Benchmark::run()
{
    if(m_first)
    {
        m_first = false;
        return true;
    }

    qint64 elapsed = m_timer.nsecsElapsed();
    if(m_count < 5 && m_step < 100000 && elapsed - m_stepStart < 100000) //0.1 ms, 太短的语句增加循环
    {
        m_count = 0;
        m_step = m_step * 10;
        m_stepStart = 0;
        m_timer.restart();
        return true;
    }

    m_count++;
    if(elapsed >= 500 * 1000000)
    {
        RunInfo info;
        info.name = m_name;
        info.time = elapsed;
        info.count = m_count * m_step;
        m_runInfo.push_back(info);
        return false;
    }
    
    m_stepStart = elapsed;
    return true;
}

//BenchmarkTest
BenchmarkTest::BenchmarkTest()
{

}

void BenchmarkTest::testBase()
{   
    return;
    QList<QVariant> varList;
    QMap<int,QVariant> varMap;
    QMap<QString,QVariant> varStrMap;

    for(int i = 0; i < 30; i++)
    {
        varList << QVariant();
        varMap[i] = QVariant();
        varStrMap[QString::number(i)] = QVariant();
    }

    m_benchmark.clear();

    JZBENCHMARK(get_list)
    {
        QVariant &v = varList[3];
    }

    JZBENCHMARK(get_int_map)
    {
        QVariant &v = varMap[3];
    }

    QVariant v_int1,v_int2,v_int3;
    v_int1 = 0;
    v_int2 = 0;
    v_int3 = 0;

    int *p_int1 = (int*)v_int1.data();
    int *p_int2 = (int*)v_int2.data();
    int *p_int3 = (int*)v_int3.data();
    JZBENCHMARK(int_add)
    {
        *p_int3 = *p_int1 + *p_int2;
    }

    JZBENCHMARK(var_int_add)
    {
        v_int3 = v_int1.toInt() + v_int2.toInt();
    }

    JZBENCHMARK(get_str_map)
    {
        QVariant &v = varStrMap["3"];
    }   

    QVariant v;
    JZBENCHMARK(variant_type)
    {
        int t = JZNodeType::variantType(v);
    }

    JZBENCHMARK(type_to_name)
    {
        QString t = JZNodeType::typeToName(Type_timer);
    }

    JZBENCHMARK(name_to_type)
    {
        int t = JZNodeType::nameToType("timer");
    }

    auto obj = JZNodeObjectManager::instance()->create(Type_intList);
    JZNodeObjectPtr ptr(obj,true);
    QVariant ptr_v = QVariant::fromValue(ptr);

    JZBENCHMARK(obj_to_variant)
    {
        QVariantList tmp;
        tmp << QVariant::fromValue(ptr);
    }

    JZBENCHMARK(variant_to_obj)
    {
        toJZObject(ptr_v);
    }

    m_benchmark.report();
}

void BenchmarkTest::testCall()
{
    return;
    if(!build())
        return;

    m_benchmark.clear();

    auto pow_func = JZNodeFunctionManager::instance()->functionImpl("pow");
    JZBENCHMARK(jz_pow)
    {
        QVariantList in,out;
        in << 0.5 << 0.6;
        m_engine.call(pow_func,in,out);
    }

    JZBENCHMARK(jz_cfunc_pow)
    {
        QVariantList in,out;
        in << 0.5 << 0.6;
        pow_func->cfunc->call(in,out);
    }

    std::function<double(double,double)> func_pow = (double (*)(double,double))(pow);
    JZBENCHMARK(jz_cfunc_pow_std)
    {
        QVariantList in,out;
        in << 0.5 << 0.6;
        out << func_pow(in[0].toDouble(),in[1].toDouble());
    }

    JZBENCHMARK(c_pow)
    {
        pow(0.5,0.6);
    }

    //list_get
    auto obj = JZNodeObjectManager::instance()->create(Type_intList);
    auto list = (JZList*)(obj->cobj());
    list->list << 1 << 2 << 3 << 4 << 5;
    JZNodeObjectPtr ptr(obj,true);

    auto list_func = JZNodeFunctionManager::instance()->functionImpl("QList<int>.get");
    JZBENCHMARK(jz_list_get)
    {
        QVariantList in,out;
        in << QVariant::fromValue(ptr) << 1;
        bool ret = m_engine.call(list_func,in,out);
        QVERIFY(ret);
    }

    JZBENCHMARK(c_list_get)
    {
        QVariant out = list->list[1];
    }

    m_benchmark.report();
}

void BenchmarkTest::testSort()
{
    JZFunctionDefine def;
    def.name = "testSort";
    def.paramIn.push_front(JZParamDefine("list",Type_intList));

    auto obj = JZNodeObjectManager::instance()->create(Type_intList);
    auto list = (JZList*)(obj->cobj());
    JZNodeObjectPtr ptr(obj,true);
    QVariantList base_list;

    int list_len = 200;
    qsrand(150);
    for(int i = 0; i < list_len; i++)
        base_list << i;

    auto script = m_file->addFunction(def);
    script->addLocalVariable("tmp",Type_int);

    auto start = script->getNode(0);

    JZNodeFor *for_i = new JZNodeFor();
    JZNodeFor *for_j = new JZNodeFor();

    JZNodeFunction *list_size = new JZNodeFunction();
    list_size->setFunction("QList<int>.size");

    JZNodeFunction *list_getI = new JZNodeFunction();
    list_getI->setFunction("QList<int>.get");

    JZNodeFunction *list_getJ = new JZNodeFunction();
    list_getJ->setFunction("QList<int>.get");

    JZNodeAdd *add = new JZNodeAdd();

    JZNodeIf *node_if = new JZNodeIf();
    JZNodeLT *node_lt = new JZNodeLT();

    JZNodeParam *param_list = new JZNodeParam();
    param_list->setVariable("list");

    JZNodeParam *param_tmp = new JZNodeParam();
    param_tmp->setVariable("tmp");

    JZNodeSetParam *set_tmp = new JZNodeSetParam();
    JZNodeFunction *set_i = new JZNodeFunction();
    JZNodeFunction *set_j = new JZNodeFunction();
    set_tmp->setVariable("tmp");
    set_i->setFunction("QList<int>.set");
    set_j->setFunction("QList<int>.set");

    script->addNode(for_i);
    script->addNode(for_j);
    script->addNode(add);
    script->addNode(param_tmp);
    script->addNode(param_list);
    script->addNode(list_size);
    script->addNode(list_getI);
    script->addNode(list_getJ);
    script->addNode(set_tmp);
    script->addNode(set_i);
    script->addNode(set_j);
    script->addNode(node_if);
    script->addNode(node_lt);

    script->addConnect(param_list->paramOutGemo(0),set_i->paramInGemo(0));
    script->addConnect(for_i->paramOutGemo(0),set_i->paramInGemo(1));
    script->addConnect(param_list->paramOutGemo(0),set_j->paramInGemo(0));
    script->addConnect(for_j->paramOutGemo(0),set_j->paramInGemo(1));

    script->addConnect(list_getI->paramOutGemo(0),node_lt->paramInGemo(0));
    script->addConnect(list_getJ->paramOutGemo(0),node_lt->paramInGemo(1));
    script->addConnect(node_lt->paramOutGemo(0),node_if->paramInGemo(0));

    script->addConnect(param_list->paramOutGemo(0),list_getI->paramInGemo(0));
    script->addConnect(for_i->paramOutGemo(0),list_getI->paramInGemo(1));
    script->addConnect(param_list->paramOutGemo(0),list_getJ->paramInGemo(0));
    script->addConnect(for_j->paramOutGemo(0),list_getJ->paramInGemo(1));

    script->addConnect(start->flowOutGemo(0),for_i->flowInGemo());
    script->addConnect(for_i->paramOutGemo(0),add->paramInGemo(0));
    add->setParamInValue(1,"1");
    script->addConnect(add->paramOutGemo(0),for_j->paramInGemo(0)); 

    script->addConnect(param_list->paramOutGemo(0),list_size->paramInGemo(0));

    script->addConnect(list_size->paramOutGemo(0),for_i->paramInGemo(2));
    script->addConnect(list_size->paramOutGemo(0),for_j->paramInGemo(2));
    
    script->addConnect(for_i->subFlowOutGemo(0),for_j->flowInGemo());

    script->addConnect(for_j->subFlowOutGemo(0),node_if->flowInGemo());

    script->addConnect(list_getJ->paramOutGemo(0),set_tmp->paramInGemo(1));
    script->addConnect(list_getI->paramOutGemo(0),set_j->paramInGemo(2));
    script->addConnect(param_tmp->paramOutGemo(0),set_i->paramInGemo(2));

    script->addConnect(node_if->subFlowOutGemo(0),set_tmp->flowInGemo());
    script->addConnect(set_tmp->flowOutGemo(0),set_j->flowInGemo());
    script->addConnect(set_j->flowOutGemo(0),set_i->flowInGemo());

    if(!build())
        return;
    
    m_benchmark.clear();

    JZBENCHMARK(jz_sort)
    {
        list->list = base_list;

        m_engine.statClear();

        QVariantList in,out;
        in << QVariant::fromValue(ptr);
        m_engine.call("testSort",in,out);

        
        m_engine.statReport();
    }

    auto clist_get = [](const QVariantList &vlist, int idx)->QVariant
    {
        return vlist[idx];
    };

    auto clist_set = [](QVariantList &vlist, int idx,const QVariant &value)
    {
        vlist[idx] = value;
    };

    JZBENCHMARK(c_list_sort)
    {
        QVariantList v_list = base_list;
        for(int i = 0; i < v_list.size(); i++)
        {
            for(int j = i+1; j < v_list.size(); j++)
            {
                QVariant v_i = clist_get(v_list,i);
                QVariant v_j = clist_get(v_list,j);

                if(v_i.toInt() < v_j.toInt())
                {
                    QVariant tmp = clist_get(v_list,j);
                    v_i = clist_get(v_list,i);
                    clist_set(v_list,j,v_i);
                    clist_set(v_list,i,tmp);
                }
            }
        }
    }

    QVector<int> c_list,c_base_list;
    qsrand(150);
    for(int i = 0; i < list_len; i++)
        c_base_list << qrand();

    JZBENCHMARK(c_sort)
    {
        c_list = c_base_list;
        for(int i = 0; i < c_list.size(); i++)
        {
            for(int j = i+1; j < c_list.size(); j++)
            {
                if(c_list[i] < c_list[j])
                {
                    int tmp = c_list[j];
                    c_list[j] = c_list[i];
                    c_list[i] = tmp;
                }
            }
        }
    }

    m_benchmark.report();
}

void BenchmarkTest::testSum()
{
    return;
    JZFunctionDefine define;
    define.name = "testSum";     
    define.paramIn.push_back(JZParamDefine("count", Type_int));   
    define.paramOut.push_back(JZParamDefine("result", Type_int64));

    JZScriptItem *script = m_file->addFunction(define);
    script->addLocalVariable(JZParamDefine("sum", Type_int64));

    JZNode *node_start = script->getNode(0);

    JZNodeFor *node_for = new JZNodeFor();
    JZNodeAdd *node_add = new JZNodeAdd();
    JZNodeParam *node_sum = new JZNodeParam();
    JZNodeParam *node_number = new JZNodeParam();
    JZNodeSetParam *node_set = new JZNodeSetParam();
    JZNodeReturn *node_ret = new JZNodeReturn();
    node_ret->setFunction(&define);

    int start_id = node_start->id();
    int for_id = script->addNode(node_for);
    int set_id = script->addNode(node_set);
    script->addNode(node_add);
    script->addNode(node_sum);
    script->addNode(node_ret);
    script->addNode(node_number);

    node_sum->setVariable("sum");
    node_set->setVariable("sum");
    node_number->setVariable("count");

    //start
    script->addConnect(JZNodeGemo(start_id, node_start->flowOut()), JZNodeGemo(for_id, node_for->flowIn()));

    script->addConnect(node_number->paramOutGemo(0), node_for->paramInGemo(2));
    script->addConnect(node_for->subFlowOutGemo(0), node_set->flowInGemo());
    script->addConnect(node_sum->paramOutGemo(0), node_add->paramInGemo(0));
    script->addConnect(node_for->paramOutGemo(0), node_add->paramInGemo(1));
    script->addConnect(node_add->paramOutGemo(0), node_set->paramInGemo(1));

    script->addConnect(node_for->flowOutGemo(0), node_ret->flowInGemo());
    script->addConnect(node_sum->paramOutGemo(0), node_ret->paramInGemo(0));

    if(!build())
        return;
    dumpAsm("testSum.asm");

    m_benchmark.clear();

    auto c_sum = [](int num)->qint64{
        qint64 sum = 0;
        for(int i = 0; i < num; i++)
            sum += i;
        return sum;
    };

    int time = 100000;
    qint64 jz_sum_count = 0;
    JZBENCHMARK(jz_sum)
    {
        QVariantList in,out;
        in << time;
        m_engine.call("testSum",in,out);
        jz_sum_count = out[0].toLongLong();
    }
    QCOMPARE(jz_sum_count,c_sum(time));
    m_engine.statReport();

    JZBENCHMARK(jz_sum_var)
    {   
        QVariant sum = (qint64)0;
        QVariant index = 0;
        QVariant step = 1;
        QVariant end = time;

        while(index.toInt() < end.toInt())
        {
            sum = m_engine.dealExpr(sum,index,OP_add);
            index = m_engine.dealExpr(index,step,OP_add);
        }
        jz_sum_count = sum.toLongLong();
    }
    QCOMPARE(jz_sum_count,c_sum(time));

    m_benchmark.report();
} 

void test_benchmark(int argc, char *argv[])
{
    BenchmarkTest test;
    QTest::qExec(&test,argc,argv);
}