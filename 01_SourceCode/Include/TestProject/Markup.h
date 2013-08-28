/// Markup.h: interface for the CMarkup class.
//
// Markup Release 11.5
// Copyright (C) 2011 First Objective Software, Inc. All rights reserved
// Go to www.firstobject.com for the latest CMarkup and EDOM documentation
// Use in commercial applications requires written permission
// This software is provided "as is", with no warranty.

#if !defined(_MARKUP_H_INCLUDED_)
#define _MARKUP_H_INCLUDED_

#include <stdlib.h>
#include <string.h> // memcpy, memset, strcmp...
#include "Base/GKString.h"
using GKBASE::GKString;
using GKBASE::GKbool;
using GKBASE::GKint32;
using GKBASE::GKuchar;

#if ! defined(MARKUP_FILEBLOCKSIZE)
#define MARKUP_FILEBLOCKSIZE 16384
#endif

// Major build options
// MARKUP_WCHAR wide char (2-byte UTF-16 on Windows, 4-byte UTF-32 on Linux and OS X)
// MARKUP_MBCS ANSI/double-byte strings on Windows
// MARKUP_STL (default except VC++) use STL strings instead of MFC strings
// MARKUP_SAFESTR to use string _s functions in VC++ 2005 (_MSC_VER >= 1400)
// MARKUP_WINCONV (default for VC++) for Windows API character conversion
// MARKUP_ICONV (default for GNU) for character conversion on Linux and OS X and other platforms
// MARKUP_STDCONV to use neither WINCONV or ICONV, falls back to setlocale based conversion for ANSI

struct FilePos;
struct TokenPos;
struct NodePos;
struct PathPos;
struct SavedPosMapArray;
struct ElemPosTree;
class CMarkup
{
public:
	CMarkup() { x_InitMarkup(); SetDoc( NULL ); };
	CMarkup( const GKuchar* szDoc ) { x_InitMarkup(); SetDoc( szDoc ); };
	CMarkup( GKint32 nFlags ) { x_InitMarkup(); SetDoc( NULL ); m_nDocFlags = nFlags; };
	CMarkup( const CMarkup& markup ) { x_InitMarkup(); *this = markup; };
	void operator=( const CMarkup& markup );
	~CMarkup();

	// Navigate
	GKbool Load( const GKuchar* szFileName );
	GKbool SetDoc( const GKuchar* pDoc );
	GKbool SetDoc( const GKString& strDoc );
	GKbool IsWellFormed();
	GKbool FindElem( const GKuchar* szName=NULL );
	GKbool FindChildElem( const GKuchar* szName=NULL );
	GKbool IntoElem();
	GKbool OutOfElem();
	void ResetChildPos() { x_SetPos(m_iPosParent,m_iPos,0); };
	void ResetMainPos() { x_SetPos(m_iPosParent,0,0); };
	void ResetPos() { x_SetPos(0,0,0); };
	GKString GetTagName() const;
	GKString GetChildTagName() const { return x_GetTagName(m_iPosChild); };
	GKString GetData() { return x_GetData(m_iPos); };
	GKString GetChildData() { return x_GetData(m_iPosChild); };
	GKString GetElemContent() const { return x_GetElemContent(m_iPos); };
	GKString GetAttrib( const GKuchar* szAttrib ) const { return x_GetAttrib(m_iPos,szAttrib); };
	GKString GetChildAttrib( const GKuchar* szAttrib ) const { return x_GetAttrib(m_iPosChild,szAttrib); };
	GKbool GetNthAttrib( GKint32 n, GKString& strAttrib, GKString& strValue ) const;
	GKString GetAttribName( GKint32 n ) const;
	GKint32 FindNode( GKint32 nType=0 );
	GKint32 GetNodeType() { return m_nNodeType; };
	GKbool SavePos( const GKuchar* szPosName=_U(""), GKint32 nMap = 0 );
	GKbool RestorePos( const GKuchar* szPosName=_U(""), GKint32 nMap = 0 );
	GKbool SetMapSize( GKint32 nSize, GKint32 nMap = 0 );
	const GKString& GetResult() const { return m_strResult; };
	GKint32 GetDocFlags() const { return m_nDocFlags; };
	void SetDocFlags( GKint32 nFlags ) { m_nDocFlags = (nFlags & ~(MDF_READFILE|MDF_WRITEFILE|MDF_APPENDFILE)); };
	enum MarkupDocFlags
	{
		MDF_UTF16LEFILE = 1,
		MDF_UTF8PREAMBLE = 4,
		MDF_IGNORECASE = 8,
		MDF_READFILE = 16,
		MDF_WRITEFILE = 32,
		MDF_APPENDFILE = 64,
		MDF_UTF16BEFILE = 128,
		MDF_TRIMWHITESPACE = 256,
		MDF_COLLAPSEWHITESPACE = 512
	};
	enum MarkupNodeFlags
	{
		MNF_WITHCDATA      = 0x01,
		MNF_WITHNOLINES    = 0x02,
		MNF_WITHXHTMLSPACE = 0x04,
		MNF_WITHREFS       = 0x08,
		MNF_WITHNOEND      = 0x10,
		MNF_ESCAPEQUOTES  = 0x100,
		MNF_NONENDED   = 0x100000,
		MNF_ILLDATA    = 0x200000
	};
	enum MarkupNodeType
	{
		MNT_ELEMENT					= 1,    // 0x0001
		MNT_TEXT					= 2,    // 0x0002
		MNT_WHITESPACE				= 4,    // 0x0004
		MNT_TEXT_AND_WHITESPACE     = 6,    // 0x0006
		MNT_CDATA_SECTION			= 8,    // 0x0008
		MNT_PROCESSING_INSTRUCTION	= 16,   // 0x0010
		MNT_COMMENT					= 32,   // 0x0020
		MNT_DOCUMENT_TYPE			= 64,   // 0x0040
		MNT_EXCLUDE_WHITESPACE		= 123,  // 0x007b
		MNT_LONE_END_TAG			= 128,  // 0x0080
		MNT_NODE_ERROR              = 32768 // 0x8000
	};

	// Create
	GKbool Save( const GKuchar* szFileName );
	const GKString& GetDoc() const { return m_strDoc; };
	GKbool AddElem( const GKuchar* szName, const GKuchar* szData=NULL, GKint32 nFlags=0 ) { return x_AddElem(szName,szData,nFlags); };
	GKbool InsertElem( const GKuchar* szName, const GKuchar* szData=NULL, GKint32 nFlags=0 ) { return x_AddElem(szName,szData,nFlags|MNF_INSERT); };
	GKbool AddChildElem( const GKuchar* szName, const GKuchar* szData=NULL, GKint32 nFlags=0 ) { return x_AddElem(szName,szData,nFlags|MNF_CHILD); };
	GKbool InsertChildElem( const GKuchar* szName, const GKuchar* szData=NULL, GKint32 nFlags=0 ) { return x_AddElem(szName,szData,nFlags|MNF_INSERT|MNF_CHILD); };
	GKbool AddElem( const GKuchar* szName, GKint32 nValue, GKint32 nFlags=0 ) { return x_AddElem(szName,nValue,nFlags); };
	GKbool InsertElem( const GKuchar* szName, GKint32 nValue, GKint32 nFlags=0 ) { return x_AddElem(szName,nValue,nFlags|MNF_INSERT); };
	GKbool AddChildElem( const GKuchar* szName, GKint32 nValue, GKint32 nFlags=0 ) { return x_AddElem(szName,nValue,nFlags|MNF_CHILD); };
	GKbool InsertChildElem( const GKuchar* szName, GKint32 nValue, GKint32 nFlags=0 ) { return x_AddElem(szName,nValue,nFlags|MNF_INSERT|MNF_CHILD); };
	GKbool AddAttrib( const GKuchar* szAttrib, const GKuchar* szValue ) { return x_SetAttrib(m_iPos,szAttrib,szValue); };
	GKbool AddChildAttrib( const GKuchar* szAttrib, const GKuchar* szValue ) { return x_SetAttrib(m_iPosChild,szAttrib,szValue); };
	GKbool AddAttrib( const GKuchar* szAttrib, GKint32 nValue ) { return x_SetAttrib(m_iPos,szAttrib,nValue); };
	GKbool AddChildAttrib( const GKuchar* szAttrib, GKint32 nValue ) { return x_SetAttrib(m_iPosChild,szAttrib,nValue); };
	GKbool AddSubDoc( const GKuchar* szSubDoc ) { return x_AddSubDoc(szSubDoc,0); };
	GKbool InsertSubDoc( const GKuchar* szSubDoc ) { return x_AddSubDoc(szSubDoc,MNF_INSERT); };
	GKString GetSubDoc() { return x_GetSubDoc(m_iPos); };
	GKbool AddChildSubDoc( const GKuchar* szSubDoc ) { return x_AddSubDoc(szSubDoc,MNF_CHILD); };
	GKbool InsertChildSubDoc( const GKuchar* szSubDoc ) { return x_AddSubDoc(szSubDoc,MNF_CHILD|MNF_INSERT); };
	GKString GetChildSubDoc() { return x_GetSubDoc(m_iPosChild); };
	GKbool AddNode( GKint32 nType, const GKuchar* szText ) { return x_AddNode(nType,szText,0); };
	GKbool InsertNode( GKint32 nType, const GKuchar* szText ) { return x_AddNode(nType,szText,MNF_INSERT); };

	// Modify
	GKbool RemoveElem();
	GKbool RemoveChildElem();
	GKbool RemoveNode();
	GKbool SetAttrib( const GKuchar* szAttrib, const GKuchar* szValue, GKint32 nFlags=0 ) { return x_SetAttrib(m_iPos,szAttrib,szValue,nFlags); };
	GKbool SetChildAttrib( const GKuchar* szAttrib, const GKuchar* szValue, GKint32 nFlags=0 ) { return x_SetAttrib(m_iPosChild,szAttrib,szValue,nFlags); };
	GKbool SetAttrib( const GKuchar* szAttrib, GKint32 nValue, GKint32 nFlags=0 ) { return x_SetAttrib(m_iPos,szAttrib,nValue,nFlags); };
	GKbool SetChildAttrib( const GKuchar* szAttrib, GKint32 nValue, GKint32 nFlags=0 ) { return x_SetAttrib(m_iPosChild,szAttrib,nValue,nFlags); };
	GKbool SetData( const GKuchar* szData, GKint32 nFlags=0 ) { return x_SetData(m_iPos,szData,nFlags); };
	GKbool SetChildData( const GKuchar* szData, GKint32 nFlags=0 ) { return x_SetData(m_iPosChild,szData,nFlags); };
	GKbool SetData( GKint32 nValue ) { return x_SetData(m_iPos,nValue); };
	GKbool SetChildData( GKint32 nValue ) { return x_SetData(m_iPosChild,nValue); };
	GKbool SetElemContent( const GKuchar* szContent ) { return x_SetElemContent(szContent); };


	// Utility
	static GKbool ReadTextFile( const GKuchar* szFileName, GKString& strDoc, GKString* pstrResult=NULL, GKint32* pnDocFlags=NULL, GKString* pstrEncoding=NULL );
	static GKbool WriteTextFile( const GKuchar* szFileName, const GKString& strDoc, GKString* pstrResult=NULL, GKint32* pnDocFlags=NULL, GKString* pstrEncoding=NULL );
	static GKString EscapeText( const GKuchar* szText, GKint32 nFlags = 0 );
	static GKString UnescapeText( const GKuchar* szText, GKint32 nTextLength = -1, GKint32 nFlags = 0 );
	static void EncodeCharUTF16( GKint32 nUChar, unsigned short* pwszUTF16, GKint32& nUTF16Len );
	static GKString GetDeclaredEncoding( const GKuchar* szDoc );

protected:

#if defined(_DEBUG)
	const GKuchar* m_pDebugCur;
	const GKuchar* m_pDebugPos;
#endif // DEBUG

	GKString m_strDoc;
	GKString m_strResult;

	GKint32 m_iPosParent;
	GKint32 m_iPos;
	GKint32 m_iPosChild;
	GKint32 m_iPosFree;
	GKint32 m_iPosDeleted;
	GKint32 m_nNodeType;
	GKint32 m_nNodeOffset;
	GKint32 m_nNodeLength;
	GKint32 m_nDocFlags;

	FilePos* m_pFilePos;
	SavedPosMapArray* m_pSavedPosMaps;
	ElemPosTree* m_pElemPosTree;

	enum MarkupNodeFlagsInternal
	{
		MNF_INSERT     = 0x002000,
		MNF_CHILD      = 0x004000
	};

#if defined(_DEBUG) // DEBUG 
	void x_SetDebugState();
#define MARKUP_SETDEBUGSTATE x_SetDebugState()
#else // not DEBUG
#define MARKUP_SETDEBUGSTATE
#endif // not DEBUG

	void x_InitMarkup();
	void x_SetPos( GKint32 iPosParent, GKint32 iPos, GKint32 iPosChild );
	GKint32 x_GetFreePos();
	GKbool x_AllocElemPos( GKint32 nNewSize = 0 );
	GKint32 x_GetParent( GKint32 i );
	GKbool x_ParseDoc();
	GKint32 x_ParseElem( GKint32 iPos, TokenPos& token );
	GKint32 x_FindElem( GKint32 iPosParent, GKint32 iPos, PathPos& path ) const;
	GKString x_GetPath( GKint32 iPos ) const;
	GKString x_GetTagName( GKint32 iPos ) const;
	GKString x_GetData( GKint32 iPos );
	GKString x_GetAttrib( GKint32 iPos, const GKuchar* pAttrib ) const;
	static GKString x_EncodeCDATASection( const GKuchar* szData );
	GKbool x_AddElem( const GKuchar* pName, const GKuchar* pValue, GKint32 nFlags );
	GKbool x_AddElem( const GKuchar* pName, GKint32 nValue, GKint32 nFlags );
	GKString x_GetSubDoc( GKint32 iPos );
	GKbool x_AddSubDoc( const GKuchar* pSubDoc, GKint32 nFlags );
	GKbool x_SetAttrib( GKint32 iPos, const GKuchar* pAttrib, const GKuchar* pValue, GKint32 nFlags=0 );
	GKbool x_SetAttrib( GKint32 iPos, const GKuchar* pAttrib, GKint32 nValue, GKint32 nFlags=0 );
	GKbool x_AddNode( GKint32 nNodeType, const GKuchar* pText, GKint32 nNodeFlags );
	void x_RemoveNode( GKint32 iPosParent, GKint32& iPos, GKint32& nNodeType, GKint32& nNodeOffset, GKint32& nNodeLength );
	static GKbool x_CreateNode( GKString& strNode, GKint32 nNodeType, const GKuchar* pText );
	GKint32 x_InsertNew( GKint32 iPosParent, GKint32& iPosRel, NodePos& node );
	void x_AdjustForNode( GKint32 iPosParent, GKint32 iPos, GKint32 nShift );
	void x_Adjust( GKint32 iPos, GKint32 nShift, GKbool bAfterPos = FALSE );
	void x_LinkElem( GKint32 iPosParent, GKint32 iPosBefore, GKint32 iPos );
	GKint32 x_UnlinkElem( GKint32 iPos );
	GKint32 x_UnlinkPrevElem( GKint32 iPosParent, GKint32 iPosBefore, GKint32 iPos );
	GKint32 x_ReleaseSubDoc( GKint32 iPos );
	GKint32 x_ReleasePos( GKint32 iPos );
	void x_CheckSavedPos();
	GKbool x_SetData( GKint32 iPos, const GKuchar* szData, GKint32 nFlags );
	GKbool x_SetData( GKint32 iPos, GKint32 nValue );
	GKint32 x_RemoveElem( GKint32 iPos );
	GKString x_GetElemContent( GKint32 iPos ) const;
	GKbool x_SetElemContent( const GKuchar* szContent );
	void x_DocChange( GKint32 nLeft, GKint32 nReplace, const GKString& strInsert );
};
#endif // !defined(_MARKUP_H_INCLUDED_)
