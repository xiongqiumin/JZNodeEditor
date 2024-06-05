#include <QDebug>
#include "ProConvert.h"

ProConvert::ProConvert()
{

}

ProConvert::~ProConvert()
{

}

void ProConvert::convert(QString code)
{
    m_code = code;
    asCScriptCode script;
    script.SetCode("main.js", code);

    asCParser parser;
    int ret = parser.ParseScript(&script);
    if (ret != 0)
        return;

    auto root = parser.GetScriptNode();
    auto child = root->firstChild;
    while (child)
    {
        if (child->nodeType == snFunction)
        {
            toFunction(child);
        }
        else if (child->nodeType == snClass)
        {
            toClass(child);
        }

        child = child->next;
    }
}

QString ProConvert::printNode(asCScriptNode *node)
{
    QString result;
    visitNode(node, 0, result);
    return result;
}

void ProConvert::visitNode(asCScriptNode *node,int level,QString &result)
{
    QString text = m_code.mid(node->tokenPos, node->tokenLength);    
    if (text.indexOf("\n") > 0)
    {
        int idx = text.indexOf("\n");
        text = text.left(idx) + "...";
    }

    QString node_type = asCScriptNode::GetDefinition(node->nodeType);
    QString token_type = asCTokenizer::GetDefinition(node->tokenType);
    QString space = QString().leftJustified(level);
    result += space + QString("[%1][%2] %3\n").arg(node_type, token_type, text);
    auto child = node->firstChild;
    if (node->firstChild)
        result += space + "{\n";

    while (child)
    {
        visitNode(child, level + 1,result);
        child = child->next;
    }

    if (node->firstChild)
        result += space + "}\n";
}

void ProConvert::toFunction(asCScriptNode *node)
{
    qDebug().noquote() << printNode(node);
}

void ProConvert::toClass(asCScriptNode *node)
{
    qDebug().noquote() << printNode(node);
}