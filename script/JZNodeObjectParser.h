#ifndef JZNODE_OBJECT_PARSER_H_
#define JZNODE_OBJECT_PARSER_H_

#include "JZNodeObject.h"

class JZNodeObjectParser
{
public:
    JZNodeObjectParser();
    ~JZNodeObjectParser();

    JZNodeObject *parse(const QString &text);    
    QString error();

protected:
    void iniContext(const QString &text);

    QChar nextChar();
    QChar readChar();
    QString readWord();    
    QVariant readVariable();    
    QVariantList *readList();
    QVariantMap *readMap();    
    JZNodeObject *readObject();
    bool readString(QString &text);
    bool readBkt(QString &context);
    bool checkIsEnd();

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
};

#endif // ! JZNODE_OBJECT_PARSER_H_
