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
    QString error();

protected:
    void iniContext(const QString &text);

    QChar nextChar();
    QChar readChar();
    QString readWord();

    QVariant readVariable();    
    JZList *readList(QString valueType,QChar start);
    JZMap *readMap(QString keyType,QString valueType);    
    JZNodeObject *readObject();
    
    bool readString(QString &text);
    bool readBkt(QString &context);
    bool checkIsEnd();
    bool checkVariable(int type,const QVariant &v);
    void makeError(const QString &error);
    void makeExpectError(QString expect,QString give);

    QList<QChar> m_gapList;
    QString m_content;
    QString m_error;
    int m_currentIndex;
    int m_line;
    int m_col;    
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
