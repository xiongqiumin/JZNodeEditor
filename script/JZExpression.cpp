#include <QRegularExpression>
#include <QStack>
#include "JZExpression.h"
#include "JZNodeType.h"

enum{
    Token_Start,
    Token_ID,
    Token_Func,
    Token_Number,    
    Token_EQ,
    Token_OP,
    Token_OP_Single,
    Token_LBkt,
    Token_RBkt,
    Token_Comma,
};

JZExpression::JZExpression()
{
    m_opList = QStringList({"+","-","*","/"});

    m_tokenMap[Token_Start] << Token_LBkt << Token_Number << Token_ID << Token_Func;
    m_tokenMap[Token_ID] << Token_OP << Token_Comma << Token_RBkt;
    m_tokenMap[Token_Func] << Token_LBkt << Token_OP << Token_Comma << Token_RBkt;
    m_tokenMap[Token_Number] << Token_OP << Token_Comma << Token_RBkt;
    m_tokenMap[Token_OP] << Token_LBkt << Token_Number << Token_ID << Token_OP_Single << Token_Func;
    m_tokenMap[Token_OP_Single] << Token_LBkt << Token_Number << Token_ID << Token_Func;
    m_tokenMap[Token_LBkt] << Token_LBkt << Token_Number << Token_ID << Token_Func;
    m_tokenMap[Token_RBkt] << Token_OP << Token_RBkt;
    m_tokenMap[Token_Comma] << Token_LBkt << Token_Number << Token_ID << Token_Func;
}

QString JZExpression::error()
{
    return m_error;
}

bool JZExpression::parse(const QString &expr)
{
    m_tokenExpr.clear();

    QRegularExpression exp_num("^(-?[0-9]+(\\.[0-9]+)?|0x[0-9a-fA-F]+)+$");
    QRegularExpression exp_id("^[a-zA-Z]+[0-9a-zA-Z]*$");

    QVector<QList<Token>> token_list;
    QList<Token> cur_token;
    QStringList lines = expr.split("\n");

    bool pre_sub_op = true;
    for(int line_idx = 0; line_idx < lines.size(); line_idx++)
    {
        QStringList strs = readToken(lines[line_idx].simplified());
        for(int i = 0; i < strs.size(); i++)
        {
            Token tk;
            tk.word = strs[i];
            if(strs[i] == "(")
                tk.type = Token_LBkt;
            else if(strs[i] == ")")
                tk.type = Token_RBkt;
            else if(m_opList.contains(tk.word))
                tk.type = Token_OP;
            else if (m_opSingleList.contains(tk.word))
                tk.type = Token_OP_Single;
            else if(tk.word == "=")
                tk.type = Token_EQ;
            else if(tk.word == ",")
                tk.type = Token_Comma;
            else if (exp_num.match(tk.word).hasMatch())
            {
                tk.type = Token_Number;
                if (pre_sub_op && exp_num.match("-" + tk.word).hasMatch())
                    cur_token.pop_back();
            }
            else if(exp_id.match(tk.word).hasMatch())
                tk.type = Token_ID;
            else
            {
                m_error = "SyntaxError: " + tk.word;
                return false;
            }
            if (pre_sub_op)
                pre_sub_op = false;
            if (tk.word == "-" && cur_token.size() > 0 && cur_token.back().type == Token_OP)
                pre_sub_op = true;
            if(tk.type == Token_LBkt && cur_token.size() > 0 && cur_token.back().type == Token_ID)
                cur_token.back().type = Token_Func;
            cur_token.push_back(tk);
        }
        if(isTokenEnd(cur_token))
        {
            token_list << cur_token;
            cur_token.clear();
        }
    }
    if(cur_token.size() > 0)
        token_list << cur_token;

    for(int i = 0; i < token_list.size(); i++)
    {
        if(!paresToken(token_list[i]))
            return false;
    }

    return true;
}

QStringList JZExpression::opList()
{
    return m_tokenExpr;
}

QStringList JZExpression::readToken(const QString &line)
{
    QStringList ret;
    QString pre;

    QStringList sep = m_opList;
    sep << "=" << "(" << ")" << ",";
    for(int i = 0; i < line.size(); i++)
    {
        if(sep.contains(line[i]))
        {
            if(pre.size() != 0)
            {
                ret << pre;
                pre.clear();
            }
            ret << line[i];
        }
        else if(line[i] != ' ')
            pre.push_back(line[i]);
    }
    if(pre.size() != 0)
        ret << pre;
    return ret;
}

bool JZExpression::isTokenEnd(const QList<Token> &tokens)
{
    if(bktNum(tokens) != 0)
        return false;
    if(tokens.size() > 0 && tokens.back().type == Token_OP)
        return false;

    return true;
}

int JZExpression::bktNum(const QList<Token> &tokens)
{
    int bkt = 0;
    for(int i = 0; i < tokens.size(); i++)
    {
        if(tokens[i].type == Token_LBkt)
            bkt++;
        else if(tokens[i].type == Token_RBkt)
            bkt--;
        if(bkt < 0)
            break;
    }
    return bkt;
}

bool JZExpression::paresToken(const QList<Token> &tokens)
{
    if(tokens.size() < 3 || tokens[0].type != Token_ID || tokens[1].type != Token_EQ)
    {
        m_error = "SyntaxError: use like a = b + c";
        return false;
    }
    if(bktNum(tokens) != 0)
    {
        m_error = "SyntaxError: bkt not match";
        return false;
    }
    const Token &last = tokens.back();
    if(last.type != Token_ID && last.type != Token_Number && last.type != Token_RBkt)
    {
        m_error = "SyntaxError: no end";
        return false;
    }

    bool new_output = false;
    if(!outList.contains(tokens[0].word))
    {
        outList.push_back(tokens[0].word);
        new_output = true;
    }

    QStack<int> func_stack;
    QStack<Token> stack;
    QList<Token> token_expr;

    QList<int> allow_type = m_tokenMap[Token_Start];
    for(int i = 2; i < tokens.size(); i++)
    {
        const Token &tk = tokens[i];
        if(!allow_type.contains(tk.type))
        {
            m_error = "SyntaxError: after " + tokens[i-1].word;
            return false;
        }

        if(tk.type == Token_ID)
        {
            if(new_output && tk.word == tokens[0].word)
            {
                m_error = "SyntaxError: use on init value " + tk.word;
                return false;
            }
            if(!inList.contains(tk.word))
                inList.push_back(tk.word);
        }

        if(tk.type == Token_OP)
        {
            int pri = tokenPri(tk);
            int top_pri = stack.size() > 0? tokenPri(stack.top()): -1;
            if(pri <= top_pri)
            {
                while(stack.size() > 0 && stack.top().type == Token_OP)
                {
                    top_pri = tokenPri(stack.top());
                    if(pri <= top_pri)
                        token_expr.push_back(stack.pop());
                    else
                        break;
                }
            }
            stack.push(tk);
        }
        else if (tk.type == Token_OP_Single)
        {
            stack.push(tk);
        }
        else if(tk.type == Token_Func)
        {
            stack.push(tk);
            func_stack.push_back(0);
        }
        else if(tk.type == Token_LBkt)
        {
            stack.push(tk);
        }
        else if(tk.type == Token_RBkt)
        {
            while(stack.size() > 0)
            {
                Token tk = stack.pop();
                if(tk.type == Token_LBkt)
                {
                    if(stack.size() > 0 && stack.back().type == Token_Func)
                    {
                        tk = stack.pop();
                        tk.word += "#" + QString::number(func_stack.pop());
                        token_expr.push_back(tk);
                    }
                    break;
                }
                else
                    token_expr.push_back(tk);
            }
        }
        else if(tk.type == Token_Comma)
        {
            while(stack.size() > 0)
            {
                Token tk = stack.back();
                if(tk.type == Token_LBkt)
                    break;
                else
                    token_expr.push_back(stack.pop());
            }
            func_stack.back()++;
        }
        else
        {
            if(func_stack.size() > 0 && func_stack.back() == 0)
                func_stack.back() = 1;

            token_expr.push_back(tk);
        }

        allow_type = m_tokenMap[tk.type];
    }

    while(stack.size() > 0)
    {
        token_expr.push_back(stack.pop());
    }

    // calc
    int regId = 0;
    for(int i = 0; i < token_expr.size(); i++)
    {
        if(token_expr[i].type == Token_OP)
        {
            auto b = stack.pop().word;
            auto a = stack.pop().word;
            QString reg = "#Reg" + QString::number(regId++);
            QString expr = reg + " = " + a + " " + token_expr[i].word + " " + b;
            m_tokenExpr.push_back(expr);

            Token ret;
            ret.type = Token_ID;
            ret.word = reg;
            stack.push(ret);
        }
        else if (token_expr[i].type == Token_OP_Single)
        {
            auto a = stack.pop().word;
            QString reg = "#Reg" + QString::number(regId++);
            QString expr = reg + " = " + token_expr[i].word + "a";
            m_tokenExpr.push_back(expr);

            Token ret;
            ret.type = Token_ID;
            ret.word = reg;
            stack.push(ret);
        }
        else if(token_expr[i].type == Token_Func)
        {
            QStringList list = token_expr[i].word.split("#");
            int num = list[1].toInt();

            QString reg = "#Reg" + QString::number(regId++);
            QString expr = reg + " = @" + list[0] + "(";
            QString param_list;
            for(int param = 0; param < num; param++)
            {
                param_list = stack.pop().word + "," + param_list;
            }
            param_list.chop(1);
            expr += param_list + ")";
            m_tokenExpr.push_back(expr);

            Token ret;
            ret.type = Token_ID;
            ret.word = reg;
            stack.push(ret);
        }
        else
            stack.push(token_expr[i]);
    }
    m_tokenExpr.push_back(tokens[0].word + " = " + stack.back().word);
    return true;
}

QString JZExpression::dump()
{
    QString log = "input:  " + inList.join(" ") + "\n";
    log += "output: " + outList.join(" ") + "\n";
    for(int i = 0; i < m_tokenExpr.size(); i++)
    {
        log += m_tokenExpr[i] + "\n";
    }
    return log;
}

int JZExpression::tokenPri(const Token &tk)
{
    return JZNodeType::opPri(tk.word);
}
