/*
   AngelCode Scripting Library
   Copyright (c) 2003-2021 Andreas Jonsson

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
// as_tokendef.h
//
// Definitions for tokens identifiable by the tokenizer
//


#ifndef AS_TOKENDEF_H
#define AS_TOKENDEF_H

#include <QString>

enum eTokenType
{
	ttUnrecognizedToken,

	ttEnd,                 // End of file

	// White space and comments
	ttWhiteSpace,          // ' ', '\t', '\r', '\n', UTF8 byte-order-mark
	ttOnelineComment,      // // \n
	ttMultilineComment,    // /* */

	// Atoms
	ttIdentifier,                  // abc123
	ttIntConstant,                 // 1234
	ttFloatConstant,               // 12.34e56f
	ttDoubleConstant,              // 12.34e56
	ttStringConstant,              // "123"
	ttMultilineStringConstant,     //
	ttNonTerminatedStringConstant, // "123
	ttBitsConstant,                // 0xFFFF

	// Math operators
	ttPlus,                // +
	ttMinus,               // -
	ttStar,                // *
	ttSlash,               // /
	ttPercent,             // %

	ttAddAssign,           // +=
	ttSubAssign,           // -=
	ttMulAssign,           // *=
	ttDivAssign,           // /=
	ttModAssign,           // %=

	ttOrAssign,            // |=
	ttAndAssign,           // &=
	ttXorAssign,           // ^=
	ttShiftLeftAssign,     // <<=
	ttShiftRightLAssign,   // >>=

	ttInc,                 // ++
	ttDec,                 // --

	ttDot,                 // .
	ttScope,               // ::

	// Statement tokens
	ttAssignment,          // =
	ttEndStatement,        // ;
	ttListSeparator,       // ,
	ttStartStatementBlock, // {
	ttEndStatementBlock,   // }
	ttOpenParenthesis,     // (
	ttCloseParenthesis,    // )
	ttOpenBracket,         // [
	ttCloseBracket,        // ]
	ttAmp,                 // &

	// Bitwise operators
	ttBitOr,               // |
	ttBitNot,              // ~
	ttBitXor,              // ^
	ttBitShiftLeft,        // <<
	ttBitShiftRight,       // >>     // TODO: In Java this is the arithmetical shift

	// Compare operators
	ttEqual,               // ==
	ttNotEqual,            // !=
	ttLessThan,            // <
	ttGreaterThan,         // >
	ttLessThanOrEqual,     // <=
	ttGreaterThanOrEqual,  // >=

	ttQuestion,            // ?
	ttColon,               // :

	// Reserved keywords
	ttIf,                  // if
	ttElse,                // else
	ttFor,                 // for
	ttWhile,               // while
	ttBool,                // bool
	ttInt,                 // int
	ttInt8,                // int8
	ttInt16,               // int16
	ttInt64,               // int64
	ttIs,                  // is
	ttNotIs,               // !is
	ttUInt,                // uint
	ttUInt8,               // uint8
	ttUInt16,              // uint16
	ttUInt64,              // uint64
	ttFloat,               // float
	ttVoid,                // void
	ttTrue,                // true
	ttFalse,               // false
	ttReturn,              // return
	ttNot,                 // not
	ttAnd,                 // and, &&
	ttOr,                  // or, ||
	ttXor,                 // xor, ^^
	ttBreak,               // break
	ttContinue,            // continue
	ttDouble,              // double
	ttSwitch,              // switch
	ttCase,                // case
	ttDefault,             // default
	ttNull,                // null
	ttCast,                // cast
	ttAuto,                // auto
	ttTry,                 // try
	ttCatch                // catch
};

struct sTokenWord
{
	const char *word;
	size_t      wordLength;
	eTokenType  tokenType;
};

#define asTokenDef(str, tok) {str, sizeof(str)-1, tok}

sTokenWord const tokenWords[] =
{
	asTokenDef("+"         , ttPlus),
	asTokenDef("+="        , ttAddAssign),
	asTokenDef("++"        , ttInc),
	asTokenDef("-"         , ttMinus),
	asTokenDef("-="        , ttSubAssign),
	asTokenDef("--"        , ttDec),
	asTokenDef("*"         , ttStar),
	asTokenDef("*="        , ttMulAssign),
	asTokenDef("/"         , ttSlash),
	asTokenDef("/="        , ttDivAssign),
	asTokenDef("%"         , ttPercent),
	asTokenDef("%="        , ttModAssign),
	asTokenDef("="         , ttAssignment),
	asTokenDef("=="        , ttEqual),
	asTokenDef("."         , ttDot),
	asTokenDef("|"         , ttBitOr),
	asTokenDef("|="        , ttOrAssign),
	asTokenDef("||"        , ttOr),
	asTokenDef("&"         , ttAmp),
	asTokenDef("&="        , ttAndAssign),
	asTokenDef("&&"        , ttAnd),
	asTokenDef("^"         , ttBitXor),
	asTokenDef("^="        , ttXorAssign),
	asTokenDef("^^"        , ttXor),
	asTokenDef("<"         , ttLessThan),
	asTokenDef("<="        , ttLessThanOrEqual),
	asTokenDef("<<"        , ttBitShiftLeft),
	asTokenDef("<<="       , ttShiftLeftAssign),
	asTokenDef(">"         , ttGreaterThan),
	asTokenDef(">="        , ttGreaterThanOrEqual),
	asTokenDef(">>"        , ttBitShiftRight),
	asTokenDef(">>="       , ttShiftRightLAssign),
	asTokenDef("~"         , ttBitNot),
	asTokenDef(";"         , ttEndStatement),
	asTokenDef(","         , ttListSeparator),
	asTokenDef("{"         , ttStartStatementBlock),
	asTokenDef("}"         , ttEndStatementBlock),
	asTokenDef("("         , ttOpenParenthesis),
	asTokenDef(")"         , ttCloseParenthesis),
	asTokenDef("["         , ttOpenBracket),
	asTokenDef("]"         , ttCloseBracket),
	asTokenDef("?"         , ttQuestion),
	asTokenDef(":"         , ttColon),
	asTokenDef("::"        , ttScope),
	asTokenDef("!"         , ttNot),
	asTokenDef("!="        , ttNotEqual),
	asTokenDef("!is"       , ttNotIs),
	asTokenDef("and"       , ttAnd),
	asTokenDef("auto"      , ttAuto),
	asTokenDef("bool"      , ttBool),
	asTokenDef("break"     , ttBreak),
	asTokenDef("case"      , ttCase),
	asTokenDef("cast"      , ttCast),
	asTokenDef("catch"     , ttCatch),
	asTokenDef("continue"  , ttContinue),
	asTokenDef("default"   , ttDefault),
	asTokenDef("double"    , ttDouble),
	asTokenDef("else"      , ttElse),
	asTokenDef("false"     , ttFalse),
	asTokenDef("float"     , ttFloat),
	asTokenDef("for"       , ttFor),
	asTokenDef("if"        , ttIf),
	asTokenDef("int"       , ttInt),
	asTokenDef("int8"      , ttInt8),
	asTokenDef("int16"     , ttInt16),
	asTokenDef("int32"     , ttInt),
	asTokenDef("int64"     , ttInt64),
	asTokenDef("is"        , ttIs),
	asTokenDef("not"       , ttNot),
	asTokenDef("null"      , ttNull),
	asTokenDef("or"        , ttOr),
	asTokenDef("return"    , ttReturn),
	asTokenDef("switch"    , ttSwitch),
	asTokenDef("true"      , ttTrue),
	asTokenDef("try"       , ttTry),
	asTokenDef("uint"      , ttUInt),
	asTokenDef("uint8"     , ttUInt8),
	asTokenDef("uint16"    , ttUInt16),
	asTokenDef("uint32"    , ttUInt),
	asTokenDef("uint64"    , ttUInt64),
	asTokenDef("void"      , ttVoid),
	asTokenDef("while"     , ttWhile),
	asTokenDef("xor"       , ttXor),
};

const unsigned int numTokenWords = sizeof(tokenWords)/sizeof(sTokenWord);

const char * const whiteSpace = " \t\r\n";

// Some keywords that are not considered tokens by the parser
// These only have meaning in specific situations. Outside these
// situations they are treated as normal identifiers.
const char * const THIS_TOKEN      = "this";
const char * const FROM_TOKEN      = "from";
const char * const SUPER_TOKEN     = "super";
const char * const SHARED_TOKEN    = "shared";
const char * const FINAL_TOKEN     = "final";
const char * const OVERRIDE_TOKEN  = "override";
const char * const GET_TOKEN       = "get";
const char * const SET_TOKEN       = "set";
const char * const ABSTRACT_TOKEN  = "abstract";
const char * const FUNCTION_TOKEN  = "function";
const char * const IF_HANDLE_TOKEN = "if_handle_then_const";
const char * const EXTERNAL_TOKEN  = "external";
const char * const EXPLICIT_TOKEN  = "explicit";
const char * const PROPERTY_TOKEN  = "property";

// Token classes
enum asETokenClass
{
	asTC_UNKNOWN = 0,
	asTC_KEYWORD = 1,
	asTC_VALUE = 2,
	asTC_IDENTIFIER = 3,
	asTC_COMMENT = 4,
	asTC_WHITESPACE = 5
};

#endif
