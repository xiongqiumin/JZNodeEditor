#include <QScopeGuard>
#include <QDebug>
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
    return QString();
}

bool JZExpressRunner::init(QString expr, QString &error, QMap<QString, int> typeMap)
{    
    return false;
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
