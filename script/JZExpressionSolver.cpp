#include <QScopeGuard>
#include "JZExpressionSolver.h"
#include "JZProject.h"
#include "JZNodeExpression.h"
#include "JZRegExpHelp.h"
#include "JZNodeEngine.h"
#include "JZNodeValue.h"
#include "JZNodeCompiler.h"

QString removeBrackets(QString input)
{
    while (input.front() == '(' && input.back() == ')')
    {
        int bkt = 0;
        int i = 0;
        for (i = 0; i < input.size(); i++)
        {
            if (input[i] == '(')
                bkt++;
            if (input[i] == ')')
            {
                bkt--;
                if (bkt == 0)
                    break;
            }
        }

        if (i == input.size() - 1)
            input = input.mid(1, input.size() - 2);
        else
            break;
    }

    auto find_pre_op = [](QString input, int idx)
    {
        int bkt = 0;
        for (int i = idx; i >= 0; i--)
        {
            if (input[i] == ')')
                bkt++;
            if (input[i] == '(')
                bkt--;
            if (bkt == 0 && input[i] == '+' || input[i] == '-')
                return 1;
            if (bkt == 0 && input[i] == '*' || input[i] == '/')
                return 2;
        }
        return 0;
    };

    auto find_next_op = [](QString input, int idx)
    {
        int bkt = 0;
        for (int i = idx; i < input.size(); i++)
        {
            if (input[i] == '(')
                bkt++;
            if (input[i] == ')')
                bkt--;
            if (bkt == 0 && (input[i] == '+' || input[i] == '-'))
                return 1;
            if (bkt == 0 && (input[i] == '*' || input[i] == '/'))
                return 2;
        }
        return 0;
    };

    int mid_op = 0;
    int i = 0;
    for (i = 0; i < input.size(); i++)
    {
        if (input[i] == '(')
        {
            if (i != 0)
            {
                mid_op = find_pre_op(input, i);
                break;
            }

            int bkt = 1;
            int j = i + 1;
            for (; j < input.size(); j++)
            {
                if (input[j] == '(')
                    bkt++;
                if (input[j] == ')')
                {
                    bkt--;
                    if (bkt == 0)
                        break;
                }
            }
            i = j + 1;
            mid_op = find_next_op(input, i);
            break;
        }
    }
    if (i == input.size())
        return input;

    int next = i;
    for (; next < input.size(); next++)
    {
        QChar c = input[next];
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == ' ')
        {
        }
        else
            break;
    }

    QString left = input.left(i);
    QString mid_str = input.mid(i, next - i);
    QString right = input.mid(next);
    QString pre_str = removeBrackets(left);
    QString next_str = removeBrackets(right);
    int pre_op = find_pre_op(pre_str, pre_str.size() - 1);
    int next_op = find_pre_op(next_str, 0);
    if (pre_op != 0 && pre_op < mid_op)
        pre_str = "(" + pre_str + ")";
    if (next_op != 0 && next_op < mid_op)
        next_str = "(" + next_str + ")";

    return pre_str + mid_str + next_str;
}

QString expressionSolver(const QString &expr_text, QString &error)
{    
    JZNodeExpression expr;
    if (!expr.setExpr(expr_text, error))
        return QString();

    QMap<QString, QString> opMap;
    QMap<QString, QString> valueMap;
    opMap["+"] = "-";
    opMap["-"] = "+";
    opMap["*"] = "/";
    opMap["/"] = "*";

    auto isLiteral = [&valueMap](QString text) {
        return valueMap.contains(text) || JZRegExpHelp::isNumber(text);
    };

    auto literalVar = [&valueMap](QString text) {
        if (valueMap.contains(text))
            return valueMap[text];
        return text;
    };

    auto ir_list = expr.irList();
    for (int i = 0; i < ir_list.size(); i++)
    {
        auto params = ir_list[i].split(" ");
        if (params.size() == 5)
        {
            bool v1 = isLiteral(params[2]);
            bool v2 = isLiteral(params[4]);
            if (v1 && v2)
            {
                valueMap[params[0]] = "(" + literalVar(params[2]) + " " + params[3] + " " + literalVar(params[4]) + ")";
            }
        }
    }

    QString resv_expr;
    for (int i = ir_list.size() - 1; i >= 0; i--)
    {
        auto params = ir_list[i].split(" ");
        if (params.size() == 3)
        {
            resv_expr = "y";
        }
        else if (params.size() == 5)
        {
            bool v1 = isLiteral(params[2]);
            bool v2 = isLiteral(params[4]);
            if (!v1 && !v2)
            {
                error = "error two variable";
                return QString();
            }

            QString op = params[3];
            if (v1 && v2)
                continue;
            else if (v1)
            {
                resv_expr = "(" + literalVar(params[2]) + " " + opMap[op] + " " + resv_expr + ")";
            }
            else
            {
                resv_expr = "(" + resv_expr + " " + opMap[op] + " " + literalVar(params[4]) + ")";
            }
        }
    }

    return removeBrackets(resv_expr);
}

bool JZExpressRunner::init(QString expr, QString &error, QMap<QString, int> typeMap)
{    
    JZScriptFile *file = new JZScriptFile();

    JZNodeExpression *node_expr = new JZNodeExpression();
    if (!node_expr->setExpr(expr, error))
    {
        delete node_expr;
        return false;
    }

    JZFunctionDefine define;
    define.name = "JZExpressRunner";
    
    auto func_file = file->addFunction(define);
    auto in_list = node_expr->paramInList();
    for (int i = 0; i < in_list.size(); i++)
    {
        auto p = node_expr->pin(in_list[i]);
        JZParamDefine param;
        param.type = p->name();
        define.paramIn.push_back(param);
    }

    auto out_list = node_expr->paramOutList();
    for (int i = 0; i < out_list.size(); i++)
    {
        auto p = node_expr->pin(in_list[i]);
        JZParamDefine param;
        param.type = p->name();
        define.paramOut.push_back(param);
    }

    auto node_start = func_file->getNode(0);    
    JZNodeReturn *node_ret = new JZNodeReturn();
    node_ret->setFunction(&define);

    func_file->addNode(node_expr);
    func_file->addNode(node_ret);
    func_file->addConnect(node_start->paramOutGemo(0), node_ret->paramInGemo(0));        

    for (int i = 0; i < in_list.size(); i++)
    {
        JZNodeParam *param = new JZNodeParam();
        param->setVariable(define.paramIn[i].name);
        func_file->addNode(param);
        func_file->addConnect(param->paramOutGemo(0), node_expr->paramInGemo(i));
    }

    for (int i = 0; i < out_list.size(); i++)
        func_file->addConnect(node_expr->paramOutGemo(i), node_ret->paramInGemo(i));

    m_script = JZNodeScriptPtr(new JZNodeScript());
    JZNodeCompiler compiler;
    if (!compiler.build(func_file, m_script.data()))
    {
        error = compiler.error();
        return false;
    }

    return true;
}

bool JZExpressRunner::call(const QVariantList &in, QVariantList &out)
{
    //g_engine->call(&m_function, in, out);
    return true;
}

void test_solver()
{    
    qDebug() << removeBrackets("(4 * 12) + 11");// == >    4 * 12 + 11
    qDebug() << removeBrackets("(1 + 2) * 3");// == >    (1 + 2) * 3
    qDebug() << removeBrackets("3 * (4 * 5)");// == >    3 * 4 * 5
    qDebug() << removeBrackets("((((523))))");// == >    523
    qDebug() << removeBrackets("(1 + 1)");// == >    1 + 1
    qDebug() << removeBrackets("1 * (2 * (3 + 4) * 5) * 6");// == >    1 * 2 * (3 + 4) * 5 * 6
    qDebug() << removeBrackets("1 * (2 + 3)");// == >    1 * (2 + 3)
    qDebug() << removeBrackets("0 * (1 + 0)");// == >    0 * (1 + 0)
    qDebug() << removeBrackets("(((2 + 92 + 82) * 46 * 70 * (24 * 62) + (94 + 25)) + 6)");// == > (2 + 92 + 82) * 46 * 70 * 24 * 62 + 94 + 25 + 6
    qDebug() << removeBrackets("(5 + 6) * (6 - 6)");
    qDebug() << removeBrackets("(5 + 6) * (6 * 6)");
    qDebug() << removeBrackets("(5 + 6) + (6 * 6)");
    qDebug() << removeBrackets("(5 + 6) + (6 - 6)");
}
