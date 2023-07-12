#ifndef JZ_EXPRESSION_H_
#define JZ_EXPRESSION_H_

#include <QStringList>
#include <QMap>

/*
    id = expr
    expr = expr op expr
        | id( expr, expr)
    op = + - * / % ^ & |

    #reg  reg
    @func
*/
class JZExpression
{
public:
    JZExpression();
    bool parse(const QString &expr);
    QStringList opList();
    QString error();
    QString dump();

    QStringList inList;
    QStringList outList;

protected:
    class Token
    {
    public:
        int type;
        QString word;
    };
    QStringList readToken(const QString &line);
    bool isTokenEnd(const QList<Token> &tokens);
    bool paresToken(const QList<Token> &tokens);
    int bktNum(const QList<Token> &tokens);
    int tokenPri(const Token &tk);

    QString m_error;
    QMap<int,QList<int>> m_tokenMap;
    QStringList m_opList;
    QStringList m_tokenExpr;
};

#endif
