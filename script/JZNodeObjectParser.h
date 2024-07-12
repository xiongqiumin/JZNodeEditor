#ifndef JZNODE_OBJECT_PARSER_H_
#define JZNODE_OBJECT_PARSER_H_

#include "JZNodeObject.h"
#include "JZContainer.h"

class JZNodeObjectParser
{
public:
    JZNodeObjectParser();
    ~JZNodeObjectParser();
    
    JZNodeObject *parse(const QString &text);
    JZNodeObject *parseToType(QString type,const QString &text);
    bool parseVariantList(const QString &format, const QString &text,QVariantList &result);
    QString error();

protected:
    struct Token
    {
        Token();

        int index;
        QString word;
    };

    bool iniContext(const QString &text);
    
    Token nextToken();
    Token readToken();
    void pushToken();

    QVariant readVariable();    
    JZList *readList(QString valueType,QString start);
    JZMap *readMap(QString keyType,QString valueType);    
    JZNodeObject *readObject();
    
    bool readBkt(QString &context);
    bool checkIsEnd();
    bool checkVariable(const QVariant &v,int type);
    void makeError(const QString &error);
    void makeExpectError(QString expect,QString give);
    void getRowCol(int index,int &row,int &col);

    QString m_content;
    QList<QChar> m_gapList;
    QList<Token> m_tokenList;
    QString m_error;
    int m_currentIndex;  
};

class JZNodeObjectFormat
{
public:
    JZNodeObjectFormat();
    ~JZNodeObjectFormat();

    QString format(JZNodeObject *obj);

protected:
    QString listToString(const JZList *list);
    QString mapToString(const JZMap *map);
    QString objectToString(JZNodeObject *obj);
    QString variantToString(const QVariant &v);
};

#endif // ! JZNODE_OBJECT_PARSER_H_
