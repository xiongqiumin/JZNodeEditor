/*
   AngelCode Scripting Library
   Copyright (c) 2003-2022 Andreas Jonsson

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
// angelscript.h
//
// The script engine interface
//


#ifndef ANGELSCRIPT_H
#define ANGELSCRIPT_H

#include <stddef.h>
#ifndef _MSC_VER
#include <stdint.h>
#endif

#ifdef AS_USE_NAMESPACE
 #define BEGIN_AS_NAMESPACE namespace AngelScript {
 #define END_AS_NAMESPACE }
 #define AS_NAMESPACE_QUALIFIER AngelScript::
#else
 #define BEGIN_AS_NAMESPACE
 #define END_AS_NAMESPACE
 #define AS_NAMESPACE_QUALIFIER ::
#endif

BEGIN_AS_NAMESPACE

// AngelScript version

#define ANGELSCRIPT_VERSION        23601
#define ANGELSCRIPT_VERSION_STRING "2.36.1"

// Data types

class asIScriptEngine;
class asIScriptModule;
class asIScriptContext;
class asIScriptGeneric;
class asIScriptObject;
class asITypeInfo;
class asIScriptFunction;
class asIBinaryStream;
class asIJITCompiler;
class asIThreadManager;
class asILockableSharedBool;
class asIStringFactory;

// Enumerations and constants

// Return codes
enum asERetCodes
{
	asSUCCESS                              =  0,
	asERROR                                = -1,
	asCONTEXT_ACTIVE                       = -2,
	asCONTEXT_NOT_FINISHED                 = -3,
	asCONTEXT_NOT_PREPARED                 = -4,
	asINVALID_ARG                          = -5,
	asNO_FUNCTION                          = -6,
	asNOT_SUPPORTED                        = -7,
	asINVALID_NAME                         = -8,
	asNAME_TAKEN                           = -9,
	asINVALID_DECLARATION                  = -10,
	asINVALID_OBJECT                       = -11,
	asINVALID_TYPE                         = -12,
	asALREADY_REGISTERED                   = -13,
	asMULTIPLE_FUNCTIONS                   = -14,
	asNO_MODULE                            = -15,
	asNO_GLOBAL_VAR                        = -16,
	asINVALID_CONFIGURATION                = -17,
	asINVALID_INTERFACE                    = -18,
	asCANT_BIND_ALL_FUNCTIONS              = -19,
	asLOWER_ARRAY_DIMENSION_NOT_REGISTERED = -20,
	asWRONG_CONFIG_GROUP                   = -21,
	asCONFIG_GROUP_IS_IN_USE               = -22,
	asILLEGAL_BEHAVIOUR_FOR_TYPE           = -23,
	asWRONG_CALLING_CONV                   = -24,
	asBUILD_IN_PROGRESS                    = -25,
	asINIT_GLOBAL_VARS_FAILED              = -26,
	asOUT_OF_MEMORY                        = -27,
	asMODULE_IS_IN_USE                     = -28
};

// Engine properties
enum asEEngineProp
{
	asEP_ALLOW_UNSAFE_REFERENCES            = 1,
	asEP_OPTIMIZE_BYTECODE                  = 2,
	asEP_COPY_SCRIPT_SECTIONS               = 3,
	asEP_MAX_STACK_SIZE                     = 4,
	asEP_USE_CHARACTER_LITERALS             = 5,
	asEP_ALLOW_MULTILINE_STRINGS            = 6,
	asEP_ALLOW_IMPLICIT_HANDLE_TYPES        = 7,
	asEP_BUILD_WITHOUT_LINE_CUES            = 8,
	asEP_INIT_GLOBAL_VARS_AFTER_BUILD       = 9,
	asEP_REQUIRE_ENUM_SCOPE                 = 10,
	asEP_SCRIPT_SCANNER                     = 11,
	asEP_INCLUDE_JIT_INSTRUCTIONS           = 12,
	asEP_STRING_ENCODING                    = 13,
	asEP_PROPERTY_ACCESSOR_MODE             = 14,
	asEP_EXPAND_DEF_ARRAY_TO_TMPL           = 15,
	asEP_AUTO_GARBAGE_COLLECT               = 16,
	asEP_DISALLOW_GLOBAL_VARS               = 17,
	asEP_ALWAYS_IMPL_DEFAULT_CONSTRUCT      = 18,
	asEP_COMPILER_WARNINGS                  = 19,
	asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE = 20,
	asEP_ALTER_SYNTAX_NAMED_ARGS            = 21,
	asEP_DISABLE_INTEGER_DIVISION           = 22,
	asEP_DISALLOW_EMPTY_LIST_ELEMENTS       = 23,
	asEP_PRIVATE_PROP_AS_PROTECTED          = 24,
	asEP_ALLOW_UNICODE_IDENTIFIERS          = 25,
	asEP_HEREDOC_TRIM_MODE                  = 26,
	asEP_MAX_NESTED_CALLS                   = 27,
	asEP_GENERIC_CALL_MODE                  = 28,
	asEP_INIT_STACK_SIZE                    = 29,
	asEP_INIT_CALL_STACK_SIZE               = 30,
	asEP_MAX_CALL_STACK_SIZE                = 31,
	asEP_IGNORE_DUPLICATE_SHARED_INTF       = 32,
	asEP_NO_DEBUG_OUTPUT                    = 33,

	asEP_LAST_PROPERTY
};

// Calling conventions
enum asECallConvTypes
{
	asCALL_CDECL             = 0,
	asCALL_STDCALL           = 1,
	asCALL_THISCALL_ASGLOBAL = 2,
	asCALL_THISCALL          = 3,
	asCALL_CDECL_OBJLAST     = 4,
	asCALL_CDECL_OBJFIRST    = 5,
	asCALL_GENERIC           = 6,
	asCALL_THISCALL_OBJLAST  = 7,
	asCALL_THISCALL_OBJFIRST = 8
};

// Object type flags
enum asEObjTypeFlags
{
	asOBJ_REF                        = (1<<0),
	asOBJ_VALUE                      = (1<<1),
	asOBJ_GC                         = (1<<2),
	asOBJ_POD                        = (1<<3),
	asOBJ_NOHANDLE                   = (1<<4),
	asOBJ_SCOPED                     = (1<<5),
	asOBJ_TEMPLATE                   = (1<<6),
	asOBJ_ASHANDLE                   = (1<<7),
	asOBJ_APP_CLASS                  = (1<<8),
	asOBJ_APP_CLASS_CONSTRUCTOR      = (1<<9),
	asOBJ_APP_CLASS_DESTRUCTOR       = (1<<10),
	asOBJ_APP_CLASS_ASSIGNMENT       = (1<<11),
	asOBJ_APP_CLASS_COPY_CONSTRUCTOR = (1<<12),
	asOBJ_APP_CLASS_C                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR),
	asOBJ_APP_CLASS_CD               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR),
	asOBJ_APP_CLASS_CA               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT),
	asOBJ_APP_CLASS_CK               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_CDA              = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT),
	asOBJ_APP_CLASS_CDK              = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_CAK              = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_CDAK             = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_CONSTRUCTOR + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_D                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR),
	asOBJ_APP_CLASS_DA               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT),
	asOBJ_APP_CLASS_DK               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_DAK              = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_DESTRUCTOR + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_A                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_ASSIGNMENT),
	asOBJ_APP_CLASS_AK               = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_ASSIGNMENT + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_K                = (asOBJ_APP_CLASS + asOBJ_APP_CLASS_COPY_CONSTRUCTOR),
	asOBJ_APP_CLASS_MORE_CONSTRUCTORS = (1<<31),
	asOBJ_APP_PRIMITIVE              = (1<<13),
	asOBJ_APP_FLOAT                  = (1<<14),
	asOBJ_APP_ARRAY                  = (1<<15),
	asOBJ_APP_CLASS_ALLINTS          = (1<<16),
	asOBJ_APP_CLASS_ALLFLOATS        = (1<<17),
	asOBJ_NOCOUNT                    = (1<<18),
	asOBJ_APP_CLASS_ALIGN8           = (1<<19),
	asOBJ_IMPLICIT_HANDLE            = (1<<20),
	asOBJ_MASK_VALID_FLAGS           = 0x801FFFFF,
	// Internal flags
	asOBJ_SCRIPT_OBJECT              = (1<<21),
	asOBJ_SHARED                     = (1<<22),
	asOBJ_NOINHERIT                  = (1<<23),
	asOBJ_FUNCDEF                    = (1<<24),
	asOBJ_LIST_PATTERN               = (1<<25),
	asOBJ_ENUM                       = (1<<26),
	asOBJ_TEMPLATE_SUBTYPE           = (1<<27),
	asOBJ_TYPEDEF                    = (1<<28),
	asOBJ_ABSTRACT                   = (1<<29),
	asOBJ_APP_ALIGN16                = (1<<30)
};

// Behaviours
enum asEBehaviours
{
	// Value object memory management
	asBEHAVE_CONSTRUCT,
	asBEHAVE_LIST_CONSTRUCT,
	asBEHAVE_DESTRUCT,

	// Reference object memory management
	asBEHAVE_FACTORY,
	asBEHAVE_LIST_FACTORY,
	asBEHAVE_ADDREF,
	asBEHAVE_RELEASE,
	asBEHAVE_GET_WEAKREF_FLAG,

	// Object operators
	asBEHAVE_TEMPLATE_CALLBACK,

	// Garbage collection behaviours
	asBEHAVE_FIRST_GC,
	 asBEHAVE_GETREFCOUNT = asBEHAVE_FIRST_GC,
	 asBEHAVE_SETGCFLAG,
	 asBEHAVE_GETGCFLAG,
	 asBEHAVE_ENUMREFS,
	 asBEHAVE_RELEASEREFS,
	asBEHAVE_LAST_GC = asBEHAVE_RELEASEREFS,

	asBEHAVE_MAX
};

// Context states
enum asEContextState
{
	asEXECUTION_FINISHED        = 0,
	asEXECUTION_SUSPENDED       = 1,
	asEXECUTION_ABORTED         = 2,
	asEXECUTION_EXCEPTION       = 3,
	asEXECUTION_PREPARED        = 4,
	asEXECUTION_UNINITIALIZED   = 5,
	asEXECUTION_ACTIVE          = 6,
	asEXECUTION_ERROR           = 7,
	asEXECUTION_DESERIALIZATION = 8
};

// Message types
enum asEMsgType
{
	asMSGTYPE_ERROR       = 0,
	asMSGTYPE_WARNING     = 1,
	asMSGTYPE_INFORMATION = 2
};

// Garbage collector flags
enum asEGCFlags
{
	asGC_FULL_CYCLE      = 1,
	asGC_ONE_STEP        = 2,
	asGC_DESTROY_GARBAGE = 4,
	asGC_DETECT_GARBAGE  = 8
};

// Token classes
enum asETokenClass
{
	asTC_UNKNOWN    = 0,
	asTC_KEYWORD    = 1,
	asTC_VALUE      = 2,
	asTC_IDENTIFIER = 3,
	asTC_COMMENT    = 4,
	asTC_WHITESPACE = 5
};

// Type id flags
enum asETypeIdFlags
{
	asTYPEID_VOID           = 0,
	asTYPEID_BOOL           = 1,
	asTYPEID_INT8           = 2,
	asTYPEID_INT16          = 3,
	asTYPEID_INT32          = 4,
	asTYPEID_INT64          = 5,
	asTYPEID_UINT8          = 6,
	asTYPEID_UINT16         = 7,
	asTYPEID_UINT32         = 8,
	asTYPEID_UINT64         = 9,
	asTYPEID_FLOAT          = 10,
	asTYPEID_DOUBLE         = 11,
	asTYPEID_OBJHANDLE      = 0x40000000,
	asTYPEID_HANDLETOCONST  = 0x20000000,
	asTYPEID_MASK_OBJECT    = 0x1C000000,
	asTYPEID_APPOBJECT      = 0x04000000,
	asTYPEID_SCRIPTOBJECT   = 0x08000000,
	asTYPEID_TEMPLATE       = 0x10000000,
	asTYPEID_MASK_SEQNBR    = 0x03FFFFFF
};

// Type modifiers
enum asETypeModifiers
{
	asTM_NONE     = 0,
	asTM_INREF    = 1,
	asTM_OUTREF   = 2,
	asTM_INOUTREF = 3,
	asTM_CONST    = 4
};

// GetModule flags
enum asEGMFlags
{
	asGM_ONLY_IF_EXISTS       = 0,
	asGM_CREATE_IF_NOT_EXISTS = 1,
	asGM_ALWAYS_CREATE        = 2
};

// Compile flags
enum asECompileFlags
{
	asCOMP_ADD_TO_MODULE = 1
};

// Function types
enum asEFuncType
{
	asFUNC_DUMMY     =-1,
	asFUNC_SYSTEM    = 0,
	asFUNC_SCRIPT    = 1,
	asFUNC_INTERFACE = 2,
	asFUNC_VIRTUAL   = 3,
	asFUNC_FUNCDEF   = 4,
	asFUNC_IMPORTED  = 5,
	asFUNC_DELEGATE  = 6
};

//
// asBYTE  =  8 bits
// asWORD  = 16 bits
// asDWORD = 32 bits
// asQWORD = 64 bits
// asPWORD = size of pointer
//
typedef signed char    asINT8;
typedef signed short   asINT16;
typedef signed int     asINT32;
typedef unsigned char  asBYTE;
typedef unsigned short asWORD;
typedef unsigned int   asUINT;
#if (defined(_MSC_VER) && _MSC_VER <= 1200) || defined(__S3E__) || (defined(_MSC_VER) && defined(__clang__))
	// size_t is not really correct, since it only guaranteed to be large enough to hold the segment size.
	// For example, on 16bit systems the size_t may be 16bits only even if pointers are 32bit. But nobody
	// is likely to use MSVC6 to compile for 16bit systems anymore, so this should be ok.
	typedef size_t         asPWORD;
#else
	typedef uintptr_t      asPWORD;
#endif
#ifdef __LP64__
	typedef unsigned int  asDWORD;
	typedef unsigned long asQWORD;
	typedef long asINT64;
#else
	typedef unsigned long asDWORD;
  #if !defined(_MSC_VER) && (defined(__GNUC__) || defined(__MWERKS__) || defined(__SUNPRO_CC) || defined(__psp2__))
	typedef uint64_t asQWORD;
	typedef int64_t asINT64;
  #else
	typedef unsigned __int64 asQWORD;
	typedef __int64 asINT64;
  #endif
#endif

// Is the target a 64bit system?
#if defined(__LP64__) || defined(__amd64__) || defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(_M_ARM64)
	#ifndef AS_64BIT_PTR
		#define AS_64BIT_PTR
	#endif
#endif

typedef void (*asFUNCTION_t)();
typedef void (*asGENFUNC_t)(asIScriptGeneric *);
typedef void *(*asALLOCFUNC_t)(size_t);
typedef void (*asFREEFUNC_t)(void *);
typedef void (*asCLEANENGINEFUNC_t)(asIScriptEngine *);
typedef void (*asCLEANMODULEFUNC_t)(asIScriptModule *);
typedef void (*asCLEANCONTEXTFUNC_t)(asIScriptContext *);
typedef void (*asCLEANFUNCTIONFUNC_t)(asIScriptFunction *);
typedef void (*asCLEANTYPEINFOFUNC_t)(asITypeInfo *);
typedef void (*asCLEANSCRIPTOBJECTFUNC_t)(asIScriptObject *);
typedef asIScriptContext *(*asREQUESTCONTEXTFUNC_t)(asIScriptEngine *, void *);
typedef void (*asRETURNCONTEXTFUNC_t)(asIScriptEngine *, asIScriptContext *, void *);
typedef void (*asCIRCULARREFFUNC_t)(asITypeInfo *, const void *, void *);

// Check if the compiler can use C++11 features
#if !defined(_MSC_VER) || _MSC_VER >= 1700   // MSVC 2012
 #if !defined(__GNUC__) || defined(__clang__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)  // gnuc 4.7 or clang
  #if !(defined(__GNUC__) && defined(__cplusplus) && __cplusplus < 201103L) // gnuc and clang require compiler flag -std=c++11
   #if !defined(__SUNPRO_CC) // Oracle Solaris Studio
    #define AS_CAN_USE_CPP11 1
   #endif
  #endif
 #endif
#endif

// This macro does basically the same thing as offsetof defined in stddef.h, but
// GNUC should not complain about the usage as I'm not using 0 as the base pointer.
#define asOFFSET(s,m) ((int)(size_t)(&reinterpret_cast<s*>(100000)->m)-100000)

#define asFUNCTION(f) asFunctionPtr(f)
#if (defined(_MSC_VER) && _MSC_VER <= 1200) || (defined(__BORLANDC__) && __BORLANDC__ < 0x590)
// MSVC 6 has a bug that prevents it from properly compiling using the correct asFUNCTIONPR with operator >
// so we need to use ordinary C style cast instead of static_cast. The drawback is that the compiler can't
// check that the cast is really valid.
// BCC v5.8 (C++Builder 2006) and earlier have a similar bug which forces us to fall back to a C-style cast.
#define asFUNCTIONPR(f,p,r) asFunctionPtr((void (*)())((r (*)p)(f)))
#else
#define asFUNCTIONPR(f,p,r) asFunctionPtr(reinterpret_cast<void (*)()>(static_cast<r (*)p>(f)))
#endif

#ifndef AS_NO_CLASS_METHODS

class asCUnknownClass;
typedef void (asCUnknownClass::*asMETHOD_t)();

struct asSFuncPtr
{
	asSFuncPtr(asBYTE f = 0)
	{
		for( size_t n = 0; n < sizeof(ptr.dummy); n++ )
			ptr.dummy[n] = 0;
		flag = f;
	}

	void CopyMethodPtr(const void *mthdPtr, size_t size)
	{
		for( size_t n = 0; n < size; n++ )
			ptr.dummy[n] = reinterpret_cast<const char *>(mthdPtr)[n];
	}

	union
	{
		// The largest known method point is 20 bytes (MSVC 64bit),
		// but with 8byte alignment this becomes 24 bytes. So we need
		// to be able to store at least that much.
		char dummy[25];
		struct {asMETHOD_t   mthd; char dummy[25-sizeof(asMETHOD_t)];} m;
		struct {asFUNCTION_t func; char dummy[25-sizeof(asFUNCTION_t)];} f;
	} ptr;
	asBYTE flag; // 1 = generic, 2 = global func, 3 = method
};

#if defined(__BORLANDC__)
// A bug in BCC (QC #85374) makes it impossible to distinguish const/non-const method overloads
// with static_cast<>. The workaround is to use an _implicit_cast instead.

 #if  __BORLANDC__ < 0x590
 // BCC v5.8 (C++Builder 2006) and earlier have an even more annoying bug which causes
 // the "pretty" workaround below (with _implicit_cast<>) to fail. For these compilers
 // we need to use a traditional C-style cast.
  #define AS_METHOD_AMBIGUITY_CAST(t) (t)
 #else
template <typename T>
  T _implicit_cast (T val)
{ return val; }
  #define AS_METHOD_AMBIGUITY_CAST(t) AS_NAMESPACE_QUALIFIER _implicit_cast<t >
 #endif
#else
 #define AS_METHOD_AMBIGUITY_CAST(t) static_cast<t >
#endif

#define asMETHOD(c,m) asSMethodPtr<sizeof(void (c::*)())>::Convert((void (c::*)())(&c::m))
#define asMETHODPR(c,m,p,r) asSMethodPtr<sizeof(void (c::*)())>::Convert(AS_METHOD_AMBIGUITY_CAST(r (c::*)p)(&c::m))

#else // Class methods are disabled

struct asSFuncPtr
{
	asSFuncPtr(asBYTE f)
	{
		for( int n = 0; n < sizeof(ptr.dummy); n++ )
			ptr.dummy[n] = 0;
		flag = f;
	}

	union
	{
		char dummy[25]; // largest known class method pointer
		struct {asFUNCTION_t func; char dummy[25-sizeof(asFUNCTION_t)];} f;
	} ptr;
	asBYTE flag; // 1 = generic, 2 = global func
};

#endif

struct asSMessageInfo
{
	const char *section;
	int         row;
	int         col;
	asEMsgType  type;
	const char *message;
};


// API functions

// ANGELSCRIPT_EXPORT is defined when compiling the dll or lib
// ANGELSCRIPT_DLL_LIBRARY_IMPORT is defined when dynamically linking to the
// dll through the link lib automatically generated by MSVC++
// ANGELSCRIPT_DLL_MANUAL_IMPORT is defined when manually loading the dll
// Don't define anything when linking statically to the lib

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
  #if defined(ANGELSCRIPT_EXPORT)
    #define AS_API __declspec(dllexport)
  #elif defined(ANGELSCRIPT_DLL_LIBRARY_IMPORT)
    #define AS_API __declspec(dllimport)
  #else // statically linked library
    #define AS_API
  #endif
#elif defined(__GNUC__)
  #if defined(ANGELSCRIPT_EXPORT)
    #define AS_API __attribute__((visibility ("default")))
  #else
    #define AS_API
  #endif
#else
  #define AS_API
#endif

END_AS_NAMESPACE

#endif
