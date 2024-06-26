/*
   AngelCode Scripting Library
   Copyright (c) 2003-2015 Andreas Jonsson

   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/


//
// as_scriptnode.cpp
//
// A node in the script tree built by the parser for compilation
//



#include "as_scriptnode.h"

BEGIN_AS_NAMESPACE

const char *asCScriptNode::GetDefinition(int nodeType)
{
    if (nodeType == snScript) return "snScript";
    else if (nodeType == snFunction) return "snFunction";
    else if (nodeType == snConstant) return "snConstant";
    else if (nodeType == snDataType) return "snDataType";
    else if (nodeType == snIdentifier) return "snIdentifier";
    else if (nodeType == snParameterList) return "snParameterList";
    else if (nodeType == snStatementBlock) return "snStatementBlock";
    else if (nodeType == snDeclaration) return "snDeclaration";
    else if (nodeType == snExpressionStatement) return "snExpressionStatement";
    else if (nodeType == snIf) return "snIf";
    else if (nodeType == snFor) return "snFor";
    else if (nodeType == snWhile) return "snWhile";
    else if (nodeType == snReturn) return "snReturn";
    else if (nodeType == snExpression) return "snExpression";
    else if (nodeType == snExprTerm) return "snExprTerm";
    else if (nodeType == snFunctionCall) return "snFunctionCall";
    else if (nodeType == snConstructCall) return "snConstructCall";
    else if (nodeType == snArgList) return "snArgList";
    else if (nodeType == snExprPreOp) return "snExprPreOp";
    else if (nodeType == snExprPostOp) return "snExprPostOp";
    else if (nodeType == snExprOperator) return "snExprOperator";
    else if (nodeType == snExprValue) return "snExprValue";
    else if (nodeType == snBreak) return "snBreak";
    else if (nodeType == snContinue) return "snContinue";
    else if (nodeType == snDoWhile) return "snDoWhile";
    else if (nodeType == snAssignment) return "snAssignment";
    else if (nodeType == snCondition) return "snCondition";
    else if (nodeType == snSwitch) return "snSwitch";
    else if (nodeType == snCase) return "snCase";
    else if (nodeType == snImport) return "snImport";
    else if (nodeType == snClass) return "snClass";
    else if (nodeType == snInitList) return "snInitList";
    else if (nodeType == snInterface) return "snInterface";
    else if (nodeType == snEnum) return "snEnum";
    else if (nodeType == snTypedef) return "snTypedef";
    else if (nodeType == snCast) return "snCast";
    else if (nodeType == snVariableAccess) return "snVariableAccess";
    else if (nodeType == snFuncDef) return "snFuncDef";
    else if (nodeType == snVirtualProperty) return "snVirtualProperty";
    else if (nodeType == snNamespace) return "snNamespace";
    else if (nodeType == snMixin) return "snMixin";
    else if (nodeType == snListPattern) return "snListPattern";
    else if (nodeType == snNamedArgument) return "snNamedArgument";
    else if (nodeType == snScope) return "snScope";
    else if (nodeType == snTryCatch) return "snTryCatch";
    else return "snUndefined";
}


asCScriptNode::asCScriptNode(eScriptNode type)
{
	nodeType    = type;
	tokenType   = ttUnrecognizedToken;
	tokenPos    = 0;
	tokenLength = 0;

	parent      = 0;
	next        = 0;
	prev        = 0;
	firstChild  = 0;
	lastChild   = 0;
}

void asCScriptNode::Destroy()
{
	// Destroy all children
	asCScriptNode *node = firstChild;
	asCScriptNode *nxt;

	while( node )
	{
		nxt = node->next;
		node->Destroy();
		node = nxt;
	}
    delete this;
}

asCScriptNode *asCScriptNode::CreateCopy()
{	
    asCScriptNode *node = new asCScriptNode(nodeType);
	node->tokenLength = tokenLength;
	node->tokenPos    = tokenPos;
	node->tokenType   = tokenType;

	asCScriptNode *child = firstChild;
	while( child )
	{
		node->AddChildLast(child->CreateCopy());
		child = child->next;
	}

	return node;
}

void asCScriptNode::SetToken(sToken *token)
{
	tokenType   = token->type;
}

void asCScriptNode::UpdateSourcePos(size_t pos, size_t length)
{
	if( pos == 0 && length == 0 ) return;

	if( tokenPos == 0 && tokenLength == 0 )
	{
		tokenPos = pos;
		tokenLength = length;
	}
	else
	{
		if( tokenPos > pos )
		{
			tokenLength = tokenPos + tokenLength - pos;
			tokenPos = pos;
		}

		if( pos + length > tokenPos + tokenLength )
		{
			tokenLength = pos + length - tokenPos;
		}
	}
}

void asCScriptNode::AddChildLast(asCScriptNode *node)
{
	// We might get a null pointer if the parser encounter an out-of-memory situation
	if( node == 0 ) return;

	if( lastChild )
	{
		lastChild->next = node;
		node->next      = 0;
		node->prev      = lastChild;
		node->parent    = this;
		lastChild       = node;
	}
	else
	{
		firstChild   = node;
		lastChild    = node;
		node->next   = 0;
		node->prev   = 0;
		node->parent = this;
	}

	UpdateSourcePos(node->tokenPos, node->tokenLength);
}

void asCScriptNode::DisconnectParent()
{
	if( parent )
	{
		if( parent->firstChild == this )
			parent->firstChild = next;
		if( parent->lastChild == this )
			parent->lastChild = prev;
	}

	if( next )
		next->prev = prev;

	if( prev )
		prev->next = next;

	parent = 0;
	next = 0;
	prev = 0;
}

END_AS_NAMESPACE

