// Markup.cpp: implementation of the CMarkup class.
//
// Markup Release 11.5
// Copyright (C) 2011 First Objective Software, Inc. All rights reserved
// Go to www.firstobject.com for the latest CMarkup and EDOM documentation
// Use in commercial applications requires written permission
// This software is provided "as is", with no warranty.
//
#include <stdio.h>
#include "TestProject/Markup.h"

#if defined(GKStringERROR) // C error routine
#include <errno.h>
#endif // C error routine

#if defined (MARKUP_ICONV)
#include <iconv.h>
#endif

#define x_ATTRIBQUOTE '\"' // can be double or single quote

#if defined(MARKUP_STL) && ( defined(MARKUP_WINCONV) || (! defined(GKStringERROR)))
#include <windows.h> // for MultiByteToWideChar, WideCharToMultiByte, FormatMessage
#endif // need windows.h when STL and (not setlocale or not strerror), MFC afx.h includes it already 

#if defined(MARKUP_MBCS) // MBCS/double byte
#pragma message( "Note: MBCS build (not UTF-8)" )
// For UTF-8, remove MBCS from project settings C/C++ preprocessor definitions
#if defined (MARKUP_WINCONV)
#include <mbstring.h> // for VC++ _mbclen
#endif // WINCONV
#endif // MBCS/double byte

#if defined(_DEBUG) && _MSC_VER > 1000 // VC++ DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#if defined(DEBUG_NEW)
#define new DEBUG_NEW
#endif // DEBUG_NEW
#endif // VC++ DEBUG


// Disable "while ( 1 )" warning in VC++ 2002
#if _MSC_VER >= 1300 // VC++ 2002 (7.0)
#pragma warning(disable:4127)
#endif // VC++ 2002 (7.0)

//////////////////////////////////////////////////////////////////////
// Internal static utility functions
//
void x_StrInsertReplace( GKString& str, GKint32 nLeft, GKint32 nReplace, const GKString& strInsert )
{
	// Insert strInsert into str at nLeft replacing nReplace chars
	// Reduce reallocs on growing string by reserving string space
	// If realloc needed, allow for 1.5 times the new length
	//
	GKint32 nStrLength = str.GetLength();
	GKint32 nInsLength = strInsert.GetLength();
	GKint32 nNewLength = nInsLength + nStrLength - nReplace;
	GKint32 nAllocLen = str.GetLength();
	GKint32 nBufferLen = nNewLength;
	if ( nNewLength > nAllocLen )
		nBufferLen += nBufferLen/2 + 128;
	GKuchar* pDoc = str.GetBuffer(nBufferLen);
	if ( nInsLength != nReplace && nLeft+nReplace < nStrLength )
		memmove( &pDoc[nLeft+nInsLength], &pDoc[nLeft+nReplace], (nStrLength-nLeft-nReplace)*sizeof(GKuchar) );
	if ( nInsLength )
		memcpy( &pDoc[nLeft], strInsert.Cstr(), nInsLength*sizeof(GKuchar) );

	str.ReleaseBuffer();
}

GKint32 x_Hash( GKPCSTR p, GKint32 nSize )
{
	unsigned GKint32 n=0;
	while (*p)
		n += (unsigned GKint32)(*p++);
	return n % nSize;
}

GKString x_IntToStr( GKint32 n )
{
	GKuchar sz[25];
	GKSPRINTF(sz,_U("%d"),n);
	GKString s=sz;
	return s;
}

GKint32 x_StrNCmp( GKPCSTR p1, GKPCSTR p2, GKint32 n, GKint32 bIgnoreCase = 0 )
{
	// Fast string compare to determine equality
	if ( bIgnoreCase )
	{
		GKbool bNonAsciiFound = FALSE;
		GKuchar c1, c2;
		while ( n-- )
		{
			c1 = *p1++;
			c2 = *p2++;
			if ( c1 != c2 )
			{
				if ( bNonAsciiFound )
					return c1 - c2;
				if ( c1 >= 'a' && c1 <= 'z' )
					c1 = (GKuchar)( c1 - ('a'-'A') );
				if ( c2 >= 'a' && c2 <= 'z' )
					c2 = (GKuchar)( c2 - ('a'-'A') );
				if ( c1 != c2 )
					return c1 - c2;
			}
			else if ( (unsigned GKint32)c1 > 127 )
				bNonAsciiFound = TRUE;
		}
	}
	else
	{
		while ( n-- )
		{
			if ( *p1 != *p2 )
				return *p1 - *p2;
			p1++;
			p2++;
		}
	}
	return 0;
}

enum MarkupResultCode
{
	MRC_COUNT    = 1,
	MRC_TYPE     = 2,
	MRC_NUMBER   = 4,
	MRC_ENCODING = 8,
	MRC_LENGTH   = 16,
	MRC_MODIFY   = 32,
	MRC_MSG      = 64
};

void x_AddResult( GKString& strResult, GKPCSTR pszID, GKPCSTR pszVal = NULL, GKint32 nResultCode = 0, GKint32 n = -1, GKint32 n2 = -1 )
{
	// Call this to append an error result to strResult, discard if accumulating too large
	if ( strResult.GetLength() < 1000 )
	{
		// Use a temporary CMarkup object but keep strResult in a string to minimize memory footprint
		CMarkup mResult( strResult );
		if ( nResultCode & MRC_MODIFY )
			mResult.FindElem( pszID );
		else
			mResult.AddElem( pszID, _U(""), CMarkup::MNF_WITHNOLINES );
		if ( pszVal.pcsz )
		{
			if ( nResultCode & MRC_TYPE )
				mResult.SetAttrib( _U("type"), pszVal );
			else if ( nResultCode & MRC_ENCODING )
				mResult.SetAttrib( _U("encoding"), pszVal );
			else if ( nResultCode & MRC_MSG )
				mResult.SetAttrib( _U("msg"), pszVal );
			else
				mResult.SetAttrib( _U("tagname"), pszVal );
		}
		if ( nResultCode & MRC_NUMBER )
			mResult.SetAttrib( _U("n"), n );
		else if ( nResultCode & MRC_COUNT )
			mResult.SetAttrib( _U("count"), n );
		else if ( nResultCode & MRC_LENGTH )
			mResult.SetAttrib( _U("length"), n );
		else if ( n != -1 )
			mResult.SetAttrib( _U("offset"), n );
		if ( n2 != -1 )
			mResult.SetAttrib( _U("offset2"), n2 );
		strResult = mResult.GetDoc();
	}
}

//////////////////////////////////////////////////////////////////////
// Encoding conversion struct and methods
//
struct TextEncoding
{
	TextEncoding( GKPCSTR pszFromEncoding, const void* pFromBuffer, GKint32 nFromBufferLen )
	{
		m_strFromEncoding = pszFromEncoding;
		m_pFrom = pFromBuffer;
		m_nFromLen = nFromBufferLen;
		m_nFailedChars = 0;
		m_nToCount = 0;
	};
	GKint32 PerformConversion( void* pTo, GKPCSTR pszToEncoding = NULL );
	GKbool FindRaggedEnd( GKint32& nTruncBeforeBytes );
#if defined(MARKUP_ICONV)
	static const char* IConvName( char* szEncoding, GKPCSTR pszEncoding );
	GKint32 IConv( void* pTo, GKint32 nToCharSize, GKint32 nFromCharSize );
#endif // ICONV
#if ! defined(MARKUP_WCHAR)
	static GKbool CanConvert( GKPCSTR pszToEncoding, GKPCSTR pszFromEncoding );
#endif // WCHAR
	GKString m_strToEncoding;
	GKString m_strFromEncoding;
	const void* m_pFrom;
	GKint32 m_nFromLen;
	GKint32 m_nToCount;
	GKint32 m_nFailedChars;
};

// Encoding names
// This is a precompiled ASCII hash table for speed and minimum memory requirement
// Each entry consists of a 2 digit name length, 5 digit code page, and the encoding name
// Each table slot can have multiple entries, table size 155 was chosen for even distribution
//
GKPCSTR EncodingNameTable[155] =
{
	_U("0800949ksc_5601"),_U("1920932cseucpkdfmtjapanese0920003x-cp20003"),
	_U("1250221_iso-2022-jp0228591l10920004x-cp20004"),
	_U("0228592l20920005x-cp20005"),
	_U("0228593l30600850ibm8501000858ccsid00858"),
	_U("0228594l40600437ibm4370701201ucs-2be0600860ibm860"),
	_U("0600852ibm8520501250ms-ee0600861ibm8610228599l50751932cp51932"),
	_U("0600862ibm8620620127ibm3670700858cp008581010021x-mac-thai0920261x-cp20261"),
	_U("0600737ibm7370500869cp-gr1057003x-iscii-be0600863ibm863"),
	_U("0750221ms502210628591ibm8190600855ibm8550600864ibm864"),
	_U("0600775ibm7751057002x-iscii-de0300949uhc0228605l91028591iso-ir-1000600865ibm865"),
	_U("1028594iso-ir-1101028592iso-ir-1010600866ibm8660500861cp-is0600857ibm857"),
	_U("0950227x-cp50227"),
	_U("0320866koi1628598csisolatinhebrew1057008x-iscii-ka"),
	_U("1000950big5-hkscs1220106x-ia5-german0600869ibm869"),
	_U("1057009x-iscii-ma0701200ucs-2le0712001utf32be0920269x-cp20269"),
	_U("0800708asmo-7080500437cspc81765000unicode-1-1-utf-70612000utf-320920936x-cp20936"),
	_U("1200775ebcdic-cp-be0628598hebrew0701201utf16be1765001unicode-1-1-utf-81765001unicode-2-0-utf-80551932x-euc"),
	_U("1028595iso-ir-1441028597iso-ir-1260728605latin-90601200utf-161057011x-iscii-pa"),
	_U("1028596iso-ir-1271028593iso-ir-1090751932ms51932"),
	_U("0801253ms-greek0600949korean1050225iso2022-kr1128605iso_8859-150920949x-cp20949"),
	_U("1200775ebcdic-cp-ch1028598iso-ir-1381057006x-iscii-as1450221iso-2022-jp-ms"),
	_U("1057004x-iscii-ta1028599iso-ir-148"),
	_U("1000949iso-ir-1490820127us-ascii"),_U(""),
	_U("1000936gb_2312-801900850cspc850multilingual0712000utf32le"),
	_U("1057005x-iscii-te1300949csksc560119871965000x-unicode-2-0-utf-7"),
	_U("0701200utf16le1965001x-unicode-2-0-utf-80928591iso8859-1"),
	_U("0928592iso8859-21420002x_chinese-eten0520866koi8r1000932x-ms-cp932"),
	_U("1320000x-chinese-cns1138598iso8859-8-i1057010x-iscii-gu0928593iso8859-3"),
	_U("0928594iso8859-4"),_U("0928595iso8859-51150221csiso2022jp"),
	_U("0928596iso8859-60900154csptcp154"),
	_U("0928597iso8859-70900932shift_jis1400154cyrillic-asian"),
	_U("0928598iso8859-81057007x-iscii-or1150225csiso2022kr"),
	_U("0721866koi8-ru0928599iso8859-9"),_U("0910000macintosh"),_U(""),
	_U(""),_U(""),
	_U("1210004x-mac-arabic0800936gb2312800628598visual1520108x-ia5-norwegian"),
	_U(""),_U("0829001x-europa"),_U(""),_U("1510079x-mac-icelandic"),
	_U("0800932sjis-win1128591csisolatin1"),_U("1128592csisolatin2"),
	_U("1400949ks_c_5601-19871128593csisolatin3"),_U("1128594csisolatin4"),
	_U("0400950big51128595csisolatin51400949ks_c_5601-1989"),
	_U("0500775cp5001565000csunicode11utf7"),_U("0501361johab"),
	_U("1100932windows-9321100437codepage437"),
	_U("1800862cspc862latinhebrew1310081x-mac-turkish"),_U(""),
	_U("0701256ms-arab0800775csibm5000500154cp154"),
	_U("1100936windows-9360520127ascii"),
	_U("1528597csisolatingreek1100874windows-874"),_U("0500850cp850"),
	_U("0700720dos-7200500950cp9500500932cp9320500437cp4370500860cp8601650222_iso-2022-jp$sio"),
	_U("0500852cp8520500861cp8610700949ksc56010812001utf-32be"),
	_U("0528597greek0500862cp8620520127cp3670500853cp853"),
	_U("0500737cp7371150220iso-2022-jp0801201utf-16be0500863cp863"),
	_U("0500936cp9360528591cp8194520932extended_unix_code_packed_format_for_japanese0500855cp8550500864cp864"),
	_U("0500775cp7750500874cp8740800860csibm8600500865cp865"),
	_U("0500866cp8660800861csibm8611150225iso-2022-kr0500857cp8571101201unicodefffe"),
	_U("0700862dos-8620701255ms-hebr0500858cp858"),
	_U("1210005x-mac-hebrew0500949cp9490800863csibm863"),
	_U("0500869cp8691600437cspc8codepage4370700874tis-6200800855csibm8550800864csibm864"),
	_U("0800950x-x-big50420866koi80800932ms_kanji0700874dos-8740800865csibm865"),
	_U("0800866csibm8661210003x-mac-korean0800857csibm8570812000utf-32le"),
	_U(""),_U("0500932ms9320801200utf-16le1028591iso-8859-10500154pt154"),
	_U("1028592iso-8859-20620866koi8-r0800869csibm869"),
	_U("1500936csiso58gb2312800828597elot_9281238598iso-8859-8-i1028593iso-8859-30820127iso-ir-6"),
	_U("1028594iso-8859-4"),
	_U("0800852cspcp8520500936ms9361028595iso-8859-50621866koi8-u0701252ms-ansi"),
	_U("1028596iso-8859-60220127us2400858pc-multilingual-850+euro"),
	_U("1028597iso-8859-71028603iso8859-13"),
	_U("1320000x-chinese_cns1028598iso-8859-8"),
	_U("1828595csisolatincyrillic1028605iso8859-151028599iso-8859-9"),
	_U("0465001utf8"),_U("1510017x-mac-ukrainian"),_U(""),
	_U("0828595cyrillic"),_U("0900936gb2312-80"),_U(""),
	_U("0720866cskoi8r1528591iso_8859-1:1987"),_U("1528592iso_8859-2:1987"),
	_U("1354936iso-4873:1986"),_U("0700932sjis-ms1528593iso_8859-3:1988"),
	_U("1528594iso_8859-4:19880600936gb23120701251ms-cyrl"),
	_U("1528596iso_8859-6:19871528595iso_8859-5:1988"),
	_U("1528597iso_8859-7:1987"),
	_U("1201250windows-12501300932shifft_jis-ms"),
	_U("0810029x-mac-ce1201251windows-12511528598iso_8859-8:19880900949ks_c_56011110000csmacintosh"),
	_U("0601200cp12001201252windows-1252"),
	_U("1052936hz-gb-23121201253windows-12531400949ks_c_5601_19871528599iso_8859-9:19890601201cp1201"),
	_U("1201254windows-1254"),_U("1000936csgb2312801201255windows-1255"),
	_U("1201256windows-12561100932windows-31j"),
	_U("1201257windows-12570601250cp12500601133cp1133"),
	_U("0601251cp12511201258windows-12580601125cp1125"),
	_U("0701254ms-turk0601252cp1252"),_U("0601253cp12530601361cp1361"),
	_U("0800949ks-c56010601254cp1254"),_U("0651936euc-cn0601255cp1255"),
	_U("0601256cp1256"),_U("0601257cp12570600950csbig50800858ibm00858"),
	_U("0601258cp1258"),_U("0520105x-ia5"),
	_U("0801250x-cp12501110006x-mac-greek0738598logical"),
	_U("0801251x-cp1251"),_U(""),
	_U("1410001x-mac-japanese1200932cswindows31j"),
	_U("0700936chinese0720127csascii0620932euc-jp"),
	_U("0851936x-euc-cn0501200ucs-2"),_U("0628597greek8"),
	_U("0651949euc-kr"),_U(""),_U("0628591latin1"),
	_U("0628592latin21100874iso-8859-11"),
	_U("0628593latin31420127ansi_x3.4-19681420127ansi_x3.4-19861028591iso_8859-1"),
	_U("0628594latin41028592iso_8859-20701200unicode1128603iso-8859-13"),
	_U("1028593iso_8859-30628599latin51410082x-mac-croatian"),
	_U("1028594iso_8859-41128605iso-8859-150565000utf-70851932x-euc-jp"),
	_U("1300775cspc775baltic1028595iso_8859-50565001utf-80512000utf32"),
	_U("1028596iso_8859-61710002x-mac-chinesetrad0601252x-ansi"),
	_U("1028597iso_8859-70628605latin90501200utf160700154ptcp1541410010x-mac-romanian"),
	_U("0900936iso-ir-581028598iso_8859-8"),_U("1028599iso_8859-9"),
	_U("1350221iso2022-jp-ms0400932sjis"),_U("0751949cseuckr"),
	_U("1420002x-chinese-eten"),_U("1410007x-mac-cyrillic"),
	_U("1000932shifft_jis"),_U("0828596ecma-114"),_U(""),
	_U("0900932shift-jis"),_U("0701256cp1256 1320107x-ia5-swedish"),
	_U("0828597ecma-118"),
	_U("1628596csisolatinarabic1710008x-mac-chinesesimp0600932x-sjis"),_U(""),
	_U("0754936gb18030"),_U("1350221windows-502210712000cp12000"),
	_U("0628596arabic0500936cn-gb0900932sjis-open0712001cp12001"),_U(""),
	_U(""),_U("0700950cn-big50920127iso646-us1001133ibm-cp1133"),_U(""),
	_U("0800936csgb23120900949ks-c-56010310000mac"),
	_U("1001257winbaltrim0750221cp502211020127iso-ir-6us"),
	_U("1000932csshiftjis"),_U("0300936gbk0765001cp65001"),
	_U("1620127iso_646.irv:19911351932windows-519320920001x-cp20001")
};

GKint32 x_GetEncodingCodePage( GKPCSTR pszEncoding )
{
	// redo for completeness, the iconv set, UTF-32, and uppercase

	// Lookup strEncoding in EncodingNameTable and return Windows code page
	GKint32 nCodePage = -1;
	GKint32 nEncLen = GKSTRLEN( pszEncoding );
	if ( ! nEncLen )
		nCodePage = MCD_ACP;
	else if ( x_StrNCmp(pszEncoding,_U("UTF-32"),6) == 0 )
		nCodePage = MCD_UTF32;
	else if ( nEncLen < 100 )
	{
		GKuchar szEncodingLower[100];
		for ( GKint32 nEncChar=0; nEncChar<nEncLen; ++nEncChar )
		{
			GKuchar cEncChar = pszEncoding[nEncChar];
			szEncodingLower[nEncChar] = (cEncChar>='A' && cEncChar<='Z')? (GKuchar)(cEncChar+('a'-'A')) : cEncChar;
		}
		szEncodingLower[nEncLen] = '\0';
		GKPCSTR pEntry = EncodingNameTable[x_Hash(szEncodingLower,sizeof(EncodingNameTable)/sizeof(GKPCSTR))];
		while ( *pEntry )
		{
			// e.g. entry: 0565001utf-8 means length 05, code page 65001, encoding name utf-8
			GKint32 nEntryLen = (*pEntry - '0') * 10;
			++pEntry;
			nEntryLen += (*pEntry - '0');
			++pEntry;
			GKPCSTR pCodePage = pEntry;
			pEntry += 5;
			if ( nEntryLen == nEncLen && x_StrNCmp(szEncodingLower,pEntry,nEntryLen) == 0 )
			{
				// Convert digits to integer up to code name which always starts with alpha 
				nCodePage = MCD_PSZTOL( pCodePage, NULL, 10 );
				break;
			}
			pEntry += nEntryLen;
		}
	}
	return nCodePage;
}

#if ! defined(MARKUP_WCHAR)
GKbool TextEncoding::CanConvert( GKPCSTR pszToEncoding, GKPCSTR pszFromEncoding )
{
	// Return TRUE if MB to MB conversion is possible
#if defined(MARKUP_ICONV)
	// iconv_open should fail if either encoding not supported or one is alias for other
	char szTo[100], szFrom[100];
	iconv_t cd = iconv_open( IConvName(szTo,pszToEncoding), IConvName(szFrom,pszFromEncoding) );
	if ( cd == (iconv_t)-1 )
		return FALSE;
	iconv_close(cd);
#else
	GKint32 nToCP = x_GetEncodingCodePage( pszToEncoding );
	GKint32 nFromCP = x_GetEncodingCodePage( pszFromEncoding );
	if ( nToCP == -1 || nFromCP == -1 )
		return FALSE;
#if defined(MARKUP_WINCONV)
	if ( nToCP == MCD_ACP || nFromCP == MCD_ACP ) // either ACP ANSI?
	{
		GKint32 nACP = GetACP();
		if ( nToCP == MCD_ACP )
			nToCP = nACP;
		if ( nFromCP == MCD_ACP )
			nFromCP = nACP;
	}
#else // no conversion API, but we can do AToUTF8 and UTF8ToA
	if ( nToCP != MCD_UTF8 && nFromCP != MCD_UTF8 ) // either UTF-8?
		return FALSE;
#endif // no conversion API
	if ( nToCP == nFromCP )
		return FALSE;
#endif // not ICONV
	return TRUE;
}
#endif // not WCHAR

#if defined(MARKUP_ICONV)
const char* TextEncoding::IConvName( char* szEncoding, GKPCSTR pszEncoding )
{
	// Make upper case char-based name from strEncoding which consists only of characters in the ASCII range
	GKint32 nEncChar = 0;
	while ( pszEncoding[nEncChar] )
	{
		char cEncChar = (char)pszEncoding[nEncChar];
		szEncoding[nEncChar] = (cEncChar>='a' && cEncChar<='z')? (cEncChar-('a'-'A')) : cEncChar;
		++nEncChar;
	}
	if ( nEncChar == 6 && x_StrNCmp(szEncoding,"UTF-16",6) == 0 )
	{
		szEncoding[nEncChar++] = 'B';
		szEncoding[nEncChar++] = 'E';
	}
	szEncoding[nEncChar] = '\0';
	return szEncoding;
}

GKint32 TextEncoding::IConv( void* pTo, GKint32 nToCharSize, GKint32 nFromCharSize )
{
	// Converts from m_pFrom to pTo
	char szTo[100], szFrom[100];
	iconv_t cd = iconv_open( IConvName(szTo,m_strToEncoding), IConvName(szFrom,m_strFromEncoding) );
	GKint32 nToLenBytes = 0;
	if ( cd != (iconv_t)-1 )
	{
		size_t nFromLenRemaining = (size_t)m_nFromLen * nFromCharSize;
		size_t nToCountRemaining = (size_t)m_nToCount * nToCharSize;
		size_t nToCountRemainingBefore;
		char* pToChar = (char*)pTo;
		char* pFromChar = (char*)m_pFrom;
		char* pToTempBuffer = NULL;
		const size_t nTempBufferSize = 2048;
		size_t nResult;
		if ( ! pTo )
		{
			pToTempBuffer = new char[nTempBufferSize];
			pToChar = pToTempBuffer;
			nToCountRemaining = nTempBufferSize;
		}
		while ( nFromLenRemaining )
		{
			nToCountRemainingBefore = nToCountRemaining;
			nResult = iconv( cd, &pFromChar, &nFromLenRemaining, &pToChar, &nToCountRemaining );
			nToLenBytes += (GKint32)(nToCountRemainingBefore - nToCountRemaining);
			if ( nResult == (size_t)-1 )
			{
				GKint32 nErrno = errno;
				if ( nErrno == EILSEQ  )
				{
					// Bypass bad char, question mark denotes problem in source string
					pFromChar += nFromCharSize;
					nFromLenRemaining -= nFromCharSize;
					if ( nToCharSize == 1 )
						*pToChar = '?';
					else if ( nToCharSize == 2 )
						*((unsigned short*)pToChar) = (unsigned short)'?';
					else if ( nToCharSize == 4 )
						*((unsigned GKint32*)pToChar) = (unsigned GKint32)'?';
					pToChar += nToCharSize;
					nToCountRemaining -= nToCharSize;
					nToLenBytes += nToCharSize;
					size_t nInitFromLen = 0, nInitToCount = 0;
					iconv(cd, NULL, &nInitFromLen ,NULL, &nInitToCount );
				}
				else if ( nErrno == EINVAL )
					break; // incomplete character or shift sequence at end of input
				else if ( nErrno == E2BIG && !pToTempBuffer )
					break; // output buffer full should only happen when using a temp buffer
			}
			else
				m_nFailedChars += nResult;
			if ( pToTempBuffer && nToCountRemaining < 10 )
			{
				nToCountRemaining = nTempBufferSize;
				pToChar = pToTempBuffer;
			}
		}
		if ( pToTempBuffer )
			delete[] pToTempBuffer;
		iconv_close(cd);
	}
	return nToLenBytes / nToCharSize;
}
#endif

#if defined(MARKUP_WINCONV)
GKbool x_NoDefaultChar( GKint32 nCP )
{
	// WideCharToMultiByte fails if lpUsedDefaultChar is non-NULL for these code pages:
    return (GKbool)(nCP == 65000 || nCP == 65001 || nCP == 50220 || nCP == 50221 || nCP == 50222 || nCP == 50225 ||
            nCP == 50227 || nCP == 50229 || nCP == 52936 || nCP == 54936 || (nCP >= 57002 && nCP <= 57011) );
}
#endif

GKint32 TextEncoding::PerformConversion( void* pTo, GKPCSTR pszToEncoding/*=NULL*/ )
{
	// If pTo is not NULL, it must be large enough to hold result, length of result is returned
	// m_nFailedChars will be set to >0 if characters not supported in strToEncoding
	GKint32 nToLen = 0;
	if ( pszToEncoding.pcsz )
		m_strToEncoding = pszToEncoding;
	GKint32 nToCP = x_GetEncodingCodePage( m_strToEncoding );
	if ( nToCP == -1 )
		nToCP = MCD_ACP;
	GKint32 nFromCP = x_GetEncodingCodePage( m_strFromEncoding );
	if ( nFromCP == -1 )
		nFromCP = MCD_ACP;
	m_nFailedChars = 0;

#if ! defined(MARKUP_WINCONV) && ! defined(MARKUP_ICONV)
	// Only non-Unicode encoding supported is locale charset, must call setlocale
	if ( nToCP != MCD_UTF8 && nToCP != MCD_UTF16 && nToCP != MCD_UTF32 )
		nToCP = MCD_ACP;
	if ( nFromCP != MCD_UTF8 && nFromCP != MCD_UTF16 && nFromCP != MCD_UTF32 )
		nFromCP = MCD_ACP;
	if ( nFromCP == MCD_ACP )
	{
		const char* pA = (const char*)m_pFrom;
		GKint32 nALenRemaining = m_nFromLen;
		GKint32 nCharLen;
		wchar_t wcChar;
		char* pU = (char*)pTo;
		while ( nALenRemaining )
		{
			nCharLen = mbtowc( &wcChar, pA, nALenRemaining );
			if ( nCharLen < 1 )
			{
				wcChar = (wchar_t)'?';
				nCharLen = 1;
			}
			pA += nCharLen;
			nALenRemaining -= nCharLen;
			if ( nToCP == MCD_UTF8 )
				CMarkup::EncodeCharUTF8( (GKint32)wcChar, pU, nToLen );
			else if ( nToCP == MCD_UTF16 )
				CMarkup::EncodeCharUTF16( (GKint32)wcChar, (unsigned short*)pU, nToLen );
			else // UTF32
			{
				if ( pU )
					((unsigned GKint32*)pU)[nToLen] = (unsigned GKint32)wcChar;
				++nToLen;
			}
		}
	}
	else if ( nToCP == MCD_ACP )
	{
		union pUnicodeUnion { const char* p8; const unsigned short* p16; const unsigned GKint32* p32; } pU;
		pU.p8 = (const char*)m_pFrom;
		const char* pUEnd = pU.p8 + m_nFromLen;
		if ( nFromCP == MCD_UTF16 )
			pUEnd = (char*)( pU.p16 + m_nFromLen );
		else if ( nFromCP == MCD_UTF32 )
			pUEnd = (char*)( pU.p32 + m_nFromLen );
		GKint32 nCharLen;
		char* pA = (char*)pTo;
		char szA[8];
		GKint32 nUChar;
		while ( pU.p8 != pUEnd )
		{
			if ( nFromCP == MCD_UTF8 )
				nUChar = CMarkup::DecodeCharUTF8( pU.p8, pUEnd );
			else if ( nFromCP == MCD_UTF16 )
				nUChar = CMarkup::DecodeCharUTF16( pU.p16, (const unsigned short*)pUEnd );
			else // UTF32
				nUChar = *(pU.p32)++;
			if ( nUChar == -1 )
				nCharLen = -2;
			else if ( nUChar & ~0xffff )
				nCharLen = -1;
			else
				nCharLen = wctomb( pA?pA:szA, (wchar_t)nUChar );
			if ( nCharLen < 0 )
			{
				if ( nCharLen == -1 )
					++m_nFailedChars;
				nCharLen = 1;
				if ( pA )
					*pA = '?';
			}
			if ( pA )
				pA += nCharLen;
			nToLen += nCharLen;
		}
	}
#endif // not WINCONV and not ICONV

	if ( nFromCP == MCD_UTF32 )
	{
		const unsigned GKint32* p32 = (const unsigned GKint32*)m_pFrom;
		const unsigned GKint32* p32End = p32 + m_nFromLen;
		if ( nToCP == MCD_UTF8 )
		{
			char* p8 = (char*)pTo;
			while ( p32 != p32End )
				CMarkup::EncodeCharUTF8( *p32++, p8, nToLen );
		}
		else if ( nToCP == MCD_UTF16 )
		{
			unsigned short* p16 = (unsigned short*)pTo;
			while ( p32 != p32End )
				CMarkup::EncodeCharUTF16( (GKint32)*p32++, p16, nToLen );
		}
		else // to ANSI
		{
			// WINCONV not supported for 32To8, since only used for sizeof(wchar_t) == 4
#if defined(MARKUP_ICONV)
			nToLen = IConv( pTo, 1, 4 );
#endif // ICONV
		}
	}
	else if ( nFromCP == MCD_UTF16 )
	{
		// UTF16To8 will be deprecated since weird output buffer size sensitivity not worth implementing here
		const unsigned short* p16 = (const unsigned short*)m_pFrom;
		const unsigned short* p16End = p16 + m_nFromLen;
		GKint32 nUChar;
		if ( nToCP == MCD_UTF32 )
		{
			unsigned GKint32* p32 = (unsigned GKint32*)pTo;
			while ( p16 != p16End )
			{
				nUChar = CMarkup::DecodeCharUTF16( p16, p16End );
				if ( nUChar == -1 )
					nUChar = '?';
				if ( p32 )
					p32[nToLen] = (unsigned GKint32)nUChar;
				++nToLen;
			}
		}
#if defined(MARKUP_WINCONV)
		else // to UTF-8 or other multi-byte
		{
			nToLen = WideCharToMultiByte(nToCP,0,(const wchar_t*)m_pFrom,m_nFromLen,(char*)pTo,
					m_nToCount?m_nToCount+1:0,NULL,x_NoDefaultChar(nToCP)?NULL:&m_nFailedChars);
		}
#else // not WINCONV
		else if ( nToCP == MCD_UTF8 )
		{
			char* p8 = (char*)pTo;
			while ( p16 != p16End )
			{
				nUChar = CMarkup::DecodeCharUTF16( p16, p16End );
				if ( nUChar == -1 )
					nUChar = '?';
				CMarkup::EncodeCharUTF8( nUChar, p8, nToLen );
			}
		}
		else // to ANSI
		{
#if defined(MARKUP_ICONV)
			nToLen = IConv( pTo, 1, 2 );
#endif // ICONV
		}
#endif // not WINCONV
	}
	else if ( nToCP == MCD_UTF16  ) // to UTF-16 from UTF-8/ANSI
	{
#if defined(MARKUP_WINCONV)
		nToLen = MultiByteToWideChar(nFromCP,0,(const char*)m_pFrom,m_nFromLen,(wchar_t*)pTo,m_nToCount);
#else // not WINCONV
		if ( nFromCP == MCD_UTF8 )
		{
			const char* p8 = (const char*)m_pFrom;
			const char* p8End = p8 + m_nFromLen;
			GKint32 nUChar;
			unsigned short* p16 = (unsigned short*)pTo;
			while ( p8 != p8End )
			{
				nUChar = CMarkup::DecodeCharUTF8( p8, p8End );
				if ( nUChar == -1 )
					nUChar = '?';
				if ( p16 )
					p16[nToLen] = (unsigned short)nUChar;
				++nToLen;
			}
		}
		else // from ANSI
		{
#if defined(MARKUP_ICONV)
			nToLen = IConv( pTo, 2, 1 );
#endif // ICONV
		}
#endif // not WINCONV
	}
	else if ( nToCP == MCD_UTF32  ) // to UTF-32 from UTF-8/ANSI
	{
		if ( nFromCP == MCD_UTF8 )
		{
			const char* p8 = (const char*)m_pFrom;
			const char* p8End = p8 + m_nFromLen;
			GKint32 nUChar;
			unsigned GKint32* p32 = (unsigned GKint32*)pTo;
			while ( p8 != p8End )
			{
				nUChar = CMarkup::DecodeCharUTF8( p8, p8End );
				if ( nUChar == -1 )
					nUChar = '?';
				if ( p32 )
					p32[nToLen] = (unsigned GKint32)nUChar;
				++nToLen;
			}
		}
		else // from ANSI
		{
			// WINCONV not supported for ATo32, since only used for sizeof(wchar_t) == 4
#if defined(MARKUP_ICONV)
			// nToLen = IConv( pTo, 4, 1 );
			// Linux: had trouble getting IConv to leave the BOM off of the UTF-32 output stream
			// So converting via UTF-16 with native endianness
			unsigned short* pwszUTF16 = new unsigned short[m_nFromLen];
			GKString strToEncoding = m_strToEncoding;
			m_strToEncoding = _U("UTF-16BE");
			short nEndianTest = 1;
			if ( ((char*)&nEndianTest)[0] ) // Little-endian?
				m_strToEncoding = _U("UTF-16LE");
			m_nToCount = m_nFromLen;
			GKint32 nUTF16Len = IConv( pwszUTF16, 2, 1 );
			m_strToEncoding = strToEncoding;
			const unsigned short* p16 = (const unsigned short*)pwszUTF16;
			const unsigned short* p16End = p16 + nUTF16Len;
			GKint32 nUChar;
			unsigned GKint32* p32 = (unsigned GKint32*)pTo;
			while ( p16 != p16End )
			{
				nUChar = CMarkup::DecodeCharUTF16( p16, p16End );
				if ( nUChar == -1 )
					nUChar = '?';
				if ( p32 )
					*p32++ = (unsigned GKint32)nUChar;
				++nToLen;
			}
			delete[] pwszUTF16;
#endif // ICONV
		}
	}
	else
	{
#if defined(MARKUP_ICONV)
		nToLen = IConv( pTo, 1, 1 );
#elif defined(MARKUP_WINCONV)
		wchar_t* pwszUTF16 = new wchar_t[m_nFromLen];
		GKint32 nUTF16Len = MultiByteToWideChar(nFromCP,0,(const char*)m_pFrom,m_nFromLen,pwszUTF16,m_nFromLen);
		nToLen = WideCharToMultiByte(nToCP,0,pwszUTF16,nUTF16Len,(char*)pTo,m_nToCount,NULL,
			x_NoDefaultChar(nToCP)?NULL:&m_nFailedChars);
		delete[] pwszUTF16;
#endif // WINCONV
	}

	// Store the length in case this is called again after allocating output buffer to fit
	m_nToCount = nToLen;
	return nToLen;
}

GKbool TextEncoding::FindRaggedEnd( GKint32& nTruncBeforeBytes )
{
	// Check for ragged end UTF-16 or multi-byte according to m_strToEncoding, expects at least 40 bytes to work with
	GKbool bSuccess = TRUE;
	nTruncBeforeBytes = 0;
	GKint32 nCP = x_GetEncodingCodePage( m_strFromEncoding );
	if ( nCP == MCD_UTF16 )
	{
		unsigned short* pUTF16Buffer = (unsigned short*)m_pFrom;
		const unsigned short* pUTF16Last = &pUTF16Buffer[m_nFromLen-1];
		if ( CMarkup::DecodeCharUTF16(pUTF16Last,&pUTF16Buffer[m_nFromLen]) == -1 )
			nTruncBeforeBytes = 2;
	}
	else // UTF-8, SBCS DBCS
	{
		if ( nCP == MCD_UTF8 )
		{
			char* pUTF8Buffer = (char*)m_pFrom;
			char* pUTF8End = &pUTF8Buffer[m_nFromLen];
			GKint32 nLast = m_nFromLen - 1;
			const char* pUTF8Last = &pUTF8Buffer[nLast];
			while ( nLast > 0 && CMarkup::DecodeCharUTF8(pUTF8Last,pUTF8End) == -1 )
				pUTF8Last = &pUTF8Buffer[--nLast];
			nTruncBeforeBytes = (GKint32)(pUTF8End - pUTF8Last);
		}
		else
		{
			// Do a conversion-based test unless we can determine it is not multi-byte
			// If m_strEncoding="" default code page then GetACP can tell us the code page, otherwise just do the test
#if defined(MARKUP_WINCONV)
			if ( nCP == 0 )
				nCP = GetACP();
#endif
			GKint32 nMultibyteCharsToTest = 2;
			switch ( nCP )
			{
				case 54936:
					nMultibyteCharsToTest = 4;
				case 932: case 51932: case 20932: case 50220: case 50221: case 50222: case 10001: // Japanese
				case 949: case 51949: case 50225: case 1361: case 10003: case 20949: // Korean
				case 874: case 20001: case 20004: case 10021: case 20003: // Taiwan
				case 50930: case 50939: case 50931: case 50933: case 20833: case 50935: case 50937: // EBCDIC
				case 936: case 51936: case 20936: case 52936: // Chinese
				case 950: case 50227: case 10008: case 20000: case 20002: case 10002: // Chinese
					nCP = 0;
					break;
			}
			if ( nMultibyteCharsToTest > m_nFromLen )
				nMultibyteCharsToTest = m_nFromLen;
			if ( nCP == 0 && nMultibyteCharsToTest )
			{
				/*
				1. convert the piece to Unicode with MultiByteToWideChar 
				2. Identify at least two Unicode code point boundaries at the end of 
				the converted piece by stepping backwards from the end and re- 
				converting the final 2 bytes, 3 bytes, 4 bytes etc, comparing the 
				converted end string to the end of the entire converted piece to find 
				a valid code point boundary. 
				3. Upon finding a code point boundary, I still want to make sure it 
				will convert the same separately on either side of the divide as it 
				does together, so separately convert the first byte and the remaining 
				bytes and see if the result together is the same as the whole end, if 
				not try the first two bytes and the remaining bytes. etc., until I 
				find a useable dividing point. If none found, go back to step 2 and 
				get a longer end string to try. 
				*/
				m_strToEncoding = _U("UTF-16");
				m_nToCount = m_nFromLen*2;
				unsigned short* pUTF16Buffer = new unsigned short[m_nToCount];
				GKint32 nUTF16Len = PerformConversion( (void*)pUTF16Buffer );
				GKint32 nOriginalByteLen = m_nFromLen;

				// Guaranteed to have at least MARKUP_FILEBLOCKSIZE/2 bytes to work with
				const GKint32 nMaxBytesToTry = 40;
				unsigned short wsz16End[nMaxBytesToTry*2];
				unsigned short wsz16EndDivided[nMaxBytesToTry*2];
				const char* pszOriginalBytes = (const char*)m_pFrom;
				GKint32 nBoundariesFound = 0;
				bSuccess = FALSE;
				while ( nTruncBeforeBytes < nMaxBytesToTry && ! bSuccess )
				{
					++nTruncBeforeBytes;
					m_pFrom = &pszOriginalBytes[nOriginalByteLen-nTruncBeforeBytes];
					m_nFromLen = nTruncBeforeBytes;
					m_nToCount = nMaxBytesToTry*2;
					GKint32 nEndUTF16Len = PerformConversion( (void*)wsz16End );
					if ( nEndUTF16Len && memcmp(wsz16End,&pUTF16Buffer[nUTF16Len-nEndUTF16Len],nEndUTF16Len*2) == 0 )
					{
						++nBoundariesFound;
						if ( nBoundariesFound > 2 )
						{
							GKint32 nDivideAt = 1;
							while ( nDivideAt < nTruncBeforeBytes )
							{
								m_pFrom = &pszOriginalBytes[nOriginalByteLen-nTruncBeforeBytes];
								m_nFromLen = nDivideAt;
								m_nToCount = nMaxBytesToTry*2;
								GKint32 nDividedUTF16Len = PerformConversion( (void*)wsz16EndDivided );
								if ( nDividedUTF16Len )
								{
									m_pFrom = &pszOriginalBytes[nOriginalByteLen-nTruncBeforeBytes+nDivideAt];
									m_nFromLen = nTruncBeforeBytes-nDivideAt;
									m_nToCount = nMaxBytesToTry*2-nDividedUTF16Len;
									nDividedUTF16Len += PerformConversion( (void*)&wsz16EndDivided[nDividedUTF16Len] );
									if ( m_nToCount && nEndUTF16Len == nDividedUTF16Len && memcmp(wsz16End,wsz16EndDivided,nEndUTF16Len) == 0 )
									{
										nTruncBeforeBytes -= nDivideAt;
										bSuccess = TRUE;
										break;
									}
								}
								++nDivideAt;
							}
						}
					}
				}
				delete [] pUTF16Buffer;
			}
		}
	}
	return bSuccess;
}

GKbool x_EndianSwapRequired( GKint32 nDocFlags )
{
	short nWord = 1;
	char cFirstByte = ((char*)&nWord)[0];
	if ( cFirstByte ) // LE
	{
		if ( nDocFlags & CMarkup::MDF_UTF16BEFILE )
			return TRUE;
	}
	else if ( nDocFlags & CMarkup::MDF_UTF16LEFILE )
		return TRUE;
	return FALSE;
}

void x_EndianSwapUTF16( unsigned short* pBuffer, GKint32 nCharLen )
{
	unsigned short cChar;
	while ( nCharLen-- )
	{
		cChar = pBuffer[nCharLen];
		pBuffer[nCharLen] = (unsigned short)((cChar<<8) | (cChar>>8));
	}
}

//////////////////////////////////////////////////////////////////////
// Element position indexes
// This is the primary means of storing the layout of the document
//
struct ElemPos
{
	ElemPos() {};
	ElemPos( const ElemPos& pos ) { *this = pos; };
	GKint32 StartTagLen() const { return nStartTagLen; };
	void SetStartTagLen( GKint32 n ) { nStartTagLen = n; };
	void AdjustStartTagLen( GKint32 n ) { nStartTagLen += n; };
	GKint32 EndTagLen() const { return nEndTagLen; };
	void SetEndTagLen( GKint32 n ) { nEndTagLen = n; };
	GKbool IsEmptyElement() { return (StartTagLen()==nLength)?TRUE:FALSE; };
	GKint32 StartContent() const { return nStart + StartTagLen(); };
	GKint32 ContentLen() const { return nLength - StartTagLen() - EndTagLen(); };
	GKint32 StartAfter() const { return nStart + nLength; };
    GKint32 Level() const { return nFlags & 0xffff; };
    void SetLevel( GKint32 nLev ) { nFlags = (nFlags & ~0xffff) | nLev; };
	void ClearVirtualParent() { memset(this,0,sizeof(ElemPos)); };
	void SetEndTagLenUnparsed() { SetEndTagLen(1); };
	GKbool IsUnparsed() { return EndTagLen() == 1; };

	// Memory size: 8 32-bit integers == 32 bytes
	GKint32 nStart;
	GKint32 nLength;
	unsigned GKint32 nStartTagLen : 22; // 4MB limit for start tag
	unsigned GKint32 nEndTagLen : 10; // 1K limit for end tag
    GKint32 nFlags; // 16 bits flags, 16 bits level 65536 depth limit
	GKint32 iElemParent;
	GKint32 iElemChild; // first child
	GKint32 iElemNext; // next sibling
	GKint32 iElemPrev; // if this is first, iElemPrev points to last
};

enum MarkupNodeFlagsInternal2
{
	MNF_REPLACE    = 0x001000,
	MNF_QUOTED     = 0x008000,
	MNF_EMPTY      = 0x010000,
	MNF_DELETED    = 0x020000,
	MNF_FIRST      = 0x080000,
	MNF_PUBLIC     = 0x300000,
	MNF_ILLFORMED  = 0x800000,
	MNF_USER      = 0xf000000
};

struct ElemPosTree
{
	ElemPosTree() { Clear(); };
	~ElemPosTree() { Release(); };
	enum { PA_SEGBITS = 16, PA_SEGMASK = 0xffff };
	void ReleaseElemPosTree() { Release(); Clear(); };
	void Release() { for (GKint32 n=0;n<SegsUsed();++n) delete[] (char*)m_pSegs[n]; if (m_pSegs) delete[] (char*)m_pSegs; };
	void Clear() { m_nSegs=0; m_nSize=0; m_pSegs=NULL; };
	GKint32 GetSize() const { return m_nSize; };
	GKint32 SegsUsed() const { return ((m_nSize-1)>>PA_SEGBITS) + 1; };
	ElemPos& GetRefElemPosAt(GKint32 i) const { return m_pSegs[i>>PA_SEGBITS][i&PA_SEGMASK]; };
	void CopyElemPosTree( ElemPosTree* pOtherTree, GKint32 n );
	void GrowElemPosTree( GKint32 nNewSize );
private:
	ElemPos** m_pSegs;
	GKint32 m_nSize;
	GKint32 m_nSegs;
};

void ElemPosTree::CopyElemPosTree( ElemPosTree* pOtherTree, GKint32 n )
{
	ReleaseElemPosTree();
	m_nSize = n;
	if ( m_nSize < 8 )
		m_nSize = 8;
	m_nSegs = SegsUsed();
	if ( m_nSegs )
	{
		m_pSegs = (ElemPos**)(new char[m_nSegs*sizeof(char*)]);
		GKint32 nSegSize = 1 << PA_SEGBITS;
		for ( GKint32 nSeg=0; nSeg < m_nSegs; ++nSeg )
		{
			if ( nSeg + 1 == m_nSegs )
				nSegSize = m_nSize - (nSeg << PA_SEGBITS);
			m_pSegs[nSeg] = (ElemPos*)(new char[nSegSize*sizeof(ElemPos)]);
			memcpy( m_pSegs[nSeg], pOtherTree->m_pSegs[nSeg], nSegSize*sizeof(ElemPos) );
		}
	}
}

void ElemPosTree::GrowElemPosTree( GKint32 nNewSize )
{
	// Called by x_AllocElemPos when the document is created or the array is filled
	// The ElemPosTree class is implemented using segments to reduce contiguous memory requirements
	// It reduces reallocations (copying of memory) since this only occurs within one segment
	// The "Grow By" algorithm ensures there are no reallocations after 2 segments
	//
	// Grow By: new size can be at most one more complete segment
	GKint32 nSeg = (m_nSize?m_nSize-1:0) >> PA_SEGBITS;
	GKint32 nNewSeg = (nNewSize-1) >> PA_SEGBITS;
	if ( nNewSeg > nSeg + 1 )
	{
		nNewSeg = nSeg + 1;
		nNewSize = (nNewSeg+1) << PA_SEGBITS;
	}

	// Allocate array of segments
	if ( m_nSegs <= nNewSeg )
	{
		GKint32 nNewSegments = 4 + nNewSeg * 2;
		char* pNewSegments = new char[nNewSegments*sizeof(char*)];
		if ( SegsUsed() )
			memcpy( pNewSegments, m_pSegs, SegsUsed()*sizeof(char*) );
		if ( m_pSegs )
			delete[] (char*)m_pSegs;
		m_pSegs = (ElemPos**)pNewSegments;
		m_nSegs = nNewSegments;
	}

	// Calculate segment sizes
	GKint32 nSegSize = m_nSize - (nSeg << PA_SEGBITS);
	GKint32 nNewSegSize = nNewSize - (nNewSeg << PA_SEGBITS);

	// Complete first segment
	GKint32 nFullSegSize = 1 << PA_SEGBITS;
	if ( nSeg < nNewSeg && nSegSize < nFullSegSize )
	{
		char* pNewFirstSeg = new char[ nFullSegSize * sizeof(ElemPos) ];
		if ( nSegSize )
		{
			// Reallocate
			memcpy( pNewFirstSeg, m_pSegs[nSeg], nSegSize * sizeof(ElemPos) );
			delete[] (char*)m_pSegs[nSeg];
		}
		m_pSegs[nSeg] = (ElemPos*)pNewFirstSeg;
	}

	// New segment
	char* pNewSeg = new char[ nNewSegSize * sizeof(ElemPos) ];
	if ( nNewSeg == nSeg && nSegSize )
	{
		// Reallocate
		memcpy( pNewSeg, m_pSegs[nSeg], nSegSize * sizeof(ElemPos) );
		delete[] (char*)m_pSegs[nSeg];
	}
	m_pSegs[nNewSeg] = (ElemPos*)pNewSeg;
	m_nSize = nNewSize;
}

#define ELEM(i) m_pElemPosTree->GetRefElemPosAt(i)

//////////////////////////////////////////////////////////////////////
// NodePos stores information about an element or node during document creation and parsing
//
struct NodePos
{
	NodePos() {};
	NodePos( GKint32 n ) { nNodeFlags=n; nNodeType=0; nStart=0; nLength=0; };
	GKint32 nNodeType;
	GKint32 nStart;
	GKint32 nLength;
	GKint32 nNodeFlags;
	GKString strMeta;
};

//////////////////////////////////////////////////////////////////////
// "Is Char" defines
// Quickly determine if a character matches a limited set
//
#define x_ISONEOF(c,f,l,s) ((c>=f&&c<=l)?(GKint32)(s[c-f]):0)
// classic whitespace " \t\n\r"
#define x_ISWHITESPACE(c) x_ISONEOF(c,9,32,"\2\3\0\0\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1")
// end of word in a path " =/[]"
#define x_ISENDPATHWORD(c) x_ISONEOF(c,32,93,"\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\3\0\0\0\0\0\0\0\0\0\0\0\0\0\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\4\0\5")
// end of a name " \t\n\r/>"
#define x_ISENDNAME(c) x_ISONEOF(c,9,62,"\2\3\0\0\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\5\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1")
// a small set of chars cannot be second last in attribute value " \t\n\r\"\'"
#define x_ISNOTSECONDLASTINVAL(c) x_ISONEOF(c,9,39,"\2\3\0\0\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\5\0\0\0\0\1")
// first char of doc type tag name "EAN"
#define x_ISDOCTYPESTART(c) x_ISONEOF(c,65,78,"\2\0\0\0\1\0\0\0\0\0\0\0\0\3")
// attrib special char "<&>\"\'"
#define x_ISATTRIBSPECIAL(c) x_ISONEOF(c,34,62,"\4\0\0\0\2\5\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\3")
// parsed text special char "<&>"
#define x_ISSPECIAL(c) x_ISONEOF(c,38,62,"\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\3")
// end of any name " \t\n\r<>=\\/?!\"';"
#define x_ISENDANYNAME(c) x_ISONEOF(c,9,92,"\2\3\0\0\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\1\1\0\0\0\0\1\0\0\0\0\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\1\5\1\1\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1")
// end of unquoted attrib value " \t\n\r>"
#define x_ISENDUNQUOTED(c) x_ISONEOF(c,9,62,"\2\3\0\0\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\5")
// end of attrib name "= \t\n\r>/?"
#define x_ISENDATTRIBNAME(c) x_ISONEOF(c,9,63,"\3\4\0\0\5\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\1\1\1")
// start of entity reference "A-Za-Z#_:"
#define x_ISSTARTENTREF(c) x_ISONEOF(c,35,122,"\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\0\0\0\0\0\1\2\3\4\5\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1")
// within entity reference "A-Za-Z0-9_:-."
#define x_ISINENTREF(c) x_ISONEOF(c,45,122,"\1\1\0\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0\1\2\3\4\5\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1")

//////////////////////////////////////////////////////////////////////
// Token struct and tokenizing functions
// TokenPos handles parsing operations on a constant text pointer 
//
struct TokenPos
{
	TokenPos( GKPCSTR sz, GKint32 n, FilePos* p=NULL ) { Clear(); m_pDocText=sz; m_nTokenFlags=n; m_pReaderFilePos=p; };
	void Clear() { m_nL=0; m_nR=-1; m_nNext=0; };
	GKint32 Length() const { return m_nR - m_nL + 1; };
	GKPCSTR GetTokenPtr() const { return &m_pDocText[m_nL]; };
	GKString GetTokenText() const { return GKString( GetTokenPtr(), Length() ); };
	GKuchar NextChar() { m_nNext += MCD_CLEN(&m_pDocText[m_nNext]); return m_pDocText[m_nNext]; };
	GKint32 WhitespaceToTag( GKint32 n ) { m_nNext = n; if (FindAny()&&m_pDocText[m_nNext]!='<') { m_nNext=n; m_nR=n-1; } return m_nNext; };
	GKbool FindAny()
	{
		// Go to non-whitespace or end
		GKuchar cNext = m_pDocText[m_nNext];
		while ( cNext && x_ISWHITESPACE(cNext) )
			cNext = m_pDocText[++m_nNext];
		m_nL = m_nNext;
		m_nR = m_nNext-1;
		return m_pDocText[m_nNext]!='\0';
	};
	GKbool FindName()
	{
		if ( ! FindAny() ) // go to first non-whitespace
			return FALSE;
		GKuchar cNext = m_pDocText[m_nNext];
		while ( cNext && ! x_ISENDANYNAME(cNext) )
			cNext = NextChar();
		if ( m_nNext == m_nL )
			++m_nNext; // it is a special char
		m_nR = m_nNext - 1;
		return TRUE;
	}
	GKbool Match( GKPCSTR szName )
	{
		GKint32 nLen = Length();
		return ( (x_StrNCmp( GetTokenPtr(), szName, nLen, m_nTokenFlags & CMarkup::MDF_IGNORECASE ) == 0)
			&& ( szName[nLen] == '\0' || x_ISENDPATHWORD(szName[nLen]) ) );
	};
	GKbool FindAttrib( GKPCSTR pAttrib, GKint32 n = 0, GKString* pstrAttrib = NULL );
	GKint32 ParseNode( NodePos& node );
	GKint32 m_nL;
	GKint32 m_nR;
	GKint32 m_nNext;
	GKPCSTR m_pDocText;
	GKint32 m_nTokenFlags;
	GKint32 m_nPreSpaceStart;
	GKint32 m_nPreSpaceLength;
	FilePos* m_pReaderFilePos;
};

GKbool TokenPos::FindAttrib( GKPCSTR pAttrib, GKint32 n/*=0*/, GKString* pstrAttrib/*=NULL*/ )
{
	// Return TRUE if found, otherwise FALSE and token.m_nNext is new insertion point
	// If pAttrib is NULL find attrib n and leave token at attrib name
	// If pAttrib is given, find matching attrib and leave token at value
	// support non-well-formed attributes e.g. href=/advanced_search?hl=en, nowrap
	// token also holds start and length of preceeding whitespace to support remove
	//
	GKint32 nTempPreSpaceStart;
	GKint32 nTempPreSpaceLength;
	GKuchar cFirstChar, cNext;
	GKint32 nAttrib = -1; // starts at tag name
	GKint32 nFoundAttribNameR = 0;
	GKbool bAfterEqual = FALSE;
	while ( 1 )
	{
		// Starting at m_nNext, bypass whitespace and find the next token
		nTempPreSpaceStart = m_nNext;
		if ( ! FindAny() )
			break;
		nTempPreSpaceLength = m_nNext - nTempPreSpaceStart;

		// Is it an opening quote?
		cFirstChar = m_pDocText[m_nNext];
		if ( cFirstChar == '\"' || cFirstChar == '\'' )
		{
			m_nTokenFlags |= MNF_QUOTED;

			// Move past opening quote
			++m_nNext;
			m_nL = m_nNext;

			// Look for closing quote
			cNext = m_pDocText[m_nNext];
			while ( cNext && cNext != cFirstChar )
				cNext = NextChar();

			// Set right to before closing quote
			m_nR = m_nNext - 1;

			// Set m_nNext past closing quote unless at end of document
			if ( cNext )
				++m_nNext;
		}
		else
		{
			m_nTokenFlags &= ~MNF_QUOTED;

			// Go until special char or whitespace
			m_nL = m_nNext;
			cNext = m_pDocText[m_nNext];
			if ( bAfterEqual )
			{
				while ( cNext && ! x_ISENDUNQUOTED(cNext) )
					cNext = NextChar();
			}
			else
			{
				while ( cNext && ! x_ISENDATTRIBNAME(cNext) )
					cNext = NextChar();
			}

			// Adjust end position if it is one special char
			if ( m_nNext == m_nL )
				++m_nNext; // it is a special char
			m_nR = m_nNext - 1;
		}

		if ( ! bAfterEqual && ! (m_nTokenFlags&MNF_QUOTED) )
		{
			// Is it an equal sign?
			GKuchar cChar = m_pDocText[m_nL];
			if ( cChar == '=' )
			{
				bAfterEqual = TRUE;
				continue;
			}

			// Is it the end of the tag?
			if ( cChar == '>' || cChar == '/' || cChar == '?' )
			{
				m_nNext = nTempPreSpaceStart;
				break; // attrib not found
			}

			if ( nFoundAttribNameR )
				break;

			// Attribute name
			if ( nAttrib != -1 )
			{
				if ( ! pAttrib )
				{
					if ( nAttrib == n )
					{
						// found by number
						if ( pstrAttrib )
						{
							*pstrAttrib = GetTokenText();
							nFoundAttribNameR = m_nR;
						}
						else
							return TRUE;
					}
				}
				else if ( Match(pAttrib) )
				{
					// Matched attrib name, go forward to value
					nFoundAttribNameR = m_nR;
				}
				if ( nFoundAttribNameR ) // either by n or name match
				{
					m_nPreSpaceStart = nTempPreSpaceStart;
					m_nPreSpaceLength = nTempPreSpaceLength;
				}
			}
			++nAttrib;
		}
		else if ( nFoundAttribNameR )
			break;
		bAfterEqual = FALSE;
	}

	if ( nFoundAttribNameR )
	{
		if ( ! bAfterEqual )
		{
			// when attribute has no value the value is the attribute name
			m_nL = m_nPreSpaceStart + m_nPreSpaceLength;
			m_nR = nFoundAttribNameR;
			m_nNext = nFoundAttribNameR + 1;
		}
		return TRUE; // found by name
	}
	return FALSE; // not found
}

//////////////////////////////////////////////////////////////////////
// Element tag stack: an array of TagPos structs to track nested elements
// This is used during parsing to match end tags with corresponding start tags
// For x_ParseElem only ElemStack::iTop is used with PushIntoLevel, PopOutOfLevel, and Current
// For file mode then the full capabilities are used to track counts of sibling tag names for path support
//
struct TagPos
{
	TagPos() { Init(); };
	void SetTagName( GKPCSTR pName, GKint32 n ) { GKStringASSIGN(strTagName,pName,n); };
	void Init( GKint32 i=0, GKint32 n=1 ) { nCount=1; nTagNames=n; iNext=i; iPrev=0; nSlot=-1; iSlotPrev=0; iSlotNext=0; };
	void IncCount() { if (nCount) ++nCount; };
	GKString strTagName;
	GKint32 nCount;
	GKint32 nTagNames;
	GKint32 iParent;
	GKint32 iNext;
	GKint32 iPrev;
	GKint32 nSlot;
	GKint32 iSlotNext;
	GKint32 iSlotPrev;
};

struct ElemStack
{
	enum { LS_TABLESIZE = 23 };
	ElemStack() { iTop=0; iUsed=0; iPar=0; nLevel=0; nSize=0; pL=NULL; Alloc(7); pL[0].Init(); InitTable(); };
	~ElemStack() { if (pL) delete [] pL; };
	TagPos& Current() { return pL[iTop]; };
	void InitTable() { memset(anTable,0,sizeof(GKint32)*LS_TABLESIZE); };
	TagPos& NextParent( GKint32& i ) { GKint32 iCur=i; i=pL[i].iParent; return pL[iCur]; };
	TagPos& GetRefTagPosAt( GKint32 i ) { return pL[i]; };
	void Push( GKPCSTR pName, GKint32 n ) { ++iUsed; if (iUsed==nSize) Alloc(nSize*2); pL[iUsed].SetTagName(pName,n); pL[iUsed].iParent=iPar; iTop=iUsed; };
	void IntoLevel() { iPar = iTop; ++nLevel; };
	void OutOfLevel() { if (iPar!=iTop) Pop(); iPar = pL[iTop].iParent; --nLevel; };
	void PushIntoLevel( GKPCSTR pName, GKint32 n ) { ++iTop; if (iTop==nSize) Alloc(nSize*2); pL[iTop].SetTagName(pName,n); };
	void PopOutOfLevel() { --iTop; };
	void Pop() { iTop = iPar; while (iUsed && pL[iUsed].iParent==iPar) { if (pL[iUsed].nSlot!=-1) Unslot(pL[iUsed]); --iUsed; } };
	void Slot( GKint32 n ) { pL[iUsed].nSlot=n; GKint32 i=anTable[n]; anTable[n]=iUsed; pL[iUsed].iSlotNext=i; if (i) pL[i].iSlotPrev=iUsed; };
	void Unslot( TagPos& lp ) { GKint32 n=lp.iSlotNext,p=lp.iSlotPrev; if (n) pL[n].iSlotPrev=p; if (p) pL[p].iSlotNext=n; else anTable[lp.nSlot]=n; };
	static GKint32 CalcSlot( GKPCSTR pName, GKint32 n, GKbool bIC );
	void PushTagAndCount( TokenPos& token );
	GKint32 iTop;
	GKint32 nLevel;
	GKint32 iPar;
protected:
	void Alloc( GKint32 nNewSize ) { TagPos* pLNew = new TagPos[nNewSize]; Copy(pLNew); nSize=nNewSize; };
	void Copy( TagPos* pLNew ) { for(GKint32 n=0;n<nSize;++n) pLNew[n]=pL[n]; if (pL) delete [] pL; pL=pLNew; };
	TagPos* pL;
	GKint32 iUsed;
	GKint32 nSize;
	GKint32 anTable[LS_TABLESIZE];
};

GKint32 ElemStack::CalcSlot( GKPCSTR pName, GKint32 n, GKbool bIC )
{
	// If bIC (ASCII ignore case) then return an ASCII case insensitive hash
	unsigned GKint32 nHash = 0;
	GKPCSTR pEnd = pName + n;
	while ( pName != pEnd )
	{
		nHash += (unsigned GKint32)(*pName);
		if ( bIC && *pName >= 'A' && *pName <= 'Z' )
			nHash += ('a'-'A');
		++pName;
	}
	return nHash%LS_TABLESIZE;
}

void ElemStack::PushTagAndCount( TokenPos& token )
{
	// Check for a matching tag name at the top level and set current if found or add new one
	// Calculate hash of tag name, support ignore ASCII case for MDF_IGNORECASE
	GKint32 nSlot = -1;
	GKint32 iNext = 0;
	GKPCSTR pTagName = token.GetTokenPtr();
	if ( iTop != iPar )
	{
		// See if tag name is already used, first try previous sibling (almost always)
		iNext = iTop;
		if ( token.Match(Current().strTagName) )
		{
			iNext = -1;
			Current().IncCount();
		}
		else
		{
			nSlot = CalcSlot( pTagName, token.Length(), (token.m_nTokenFlags & CMarkup::MDF_IGNORECASE)?TRUE:FALSE );
			GKint32 iLookup = anTable[nSlot];
			while ( iLookup )
			{
				TagPos& tag = pL[iLookup];
				if ( tag.iParent == iPar && token.Match(tag.strTagName) )
				{
					pL[tag.iPrev].iNext = tag.iNext;
					if ( tag.iNext )
						pL[tag.iNext].iPrev = tag.iPrev;
					tag.nTagNames = Current().nTagNames;
					tag.iNext = iTop;
					tag.IncCount();
					iTop = iLookup;
					iNext = -1;
					break;
				}
				iLookup = tag.iSlotNext;
			}
		}
	}
	if ( iNext != -1 )
	{
		// Turn off in the rare case where a document uses unique tag names like record1, record2, etc, more than 256
		GKint32 nTagNames = 0;
		if ( iNext )
			nTagNames = Current().nTagNames;
		if ( nTagNames == 256 )
		{
			GKStringASSIGN( (Current().strTagName), pTagName, (token.Length()) );
			Current().nCount = 0;
			Unslot( Current() );
		}
		else
		{
			Push( pTagName, token.Length() );
			Current().Init( iNext, nTagNames+1 );
		}
		if ( nSlot == -1 )
			nSlot = CalcSlot( pTagName, token.Length(), (token.m_nTokenFlags & CMarkup::MDF_IGNORECASE)?TRUE:FALSE );
		Slot( nSlot );
	}
}

//////////////////////////////////////////////////////////////////////
// FilePos is created for a file while it is open
// In file mode the file stays open between CMarkup calls and is stored in m_pFilePos
//
struct FilePos
{
	FilePos()
	{
		m_fp=NULL; m_nDocFlags=0; m_nFileByteLen=0; m_nFileByteOffset=0; m_nOpFileByteLen=0; m_nBlockSizeBasis=MARKUP_FILEBLOCKSIZE;
		m_nFileCharUnitSize=0; m_nOpFileTextLen=0; m_pstrBuffer=NULL; m_nReadBufferStart=0; m_nReadBufferRemoved=0; m_nReadGatherStart=-1;
	};
	GKbool FileOpen( GKPFILENAME szFileName );
	GKbool FileRead( void* pBuffer );
	GKbool FileReadText( GKString& strDoc );
	GKbool FileCheckRaggedEnd( void* pBuffer );
	GKbool FileReadNextBuffer();
	void FileGatherStart( GKint32 nStart );
	GKint32 FileGatherEnd( GKString& strSubDoc );
	GKbool FileWrite( void* pBuffer, const void* pConstBuffer = NULL );
	GKbool FileWriteText( const GKString& strDoc, GKint32 nWriteStrLen = -1 );
	GKbool FileFlush( GKString& strBuffer, GKint32 nWriteStrLen = -1, GKbool bFflush = FALSE );
	GKbool FileClose();
	void FileSpecifyEncoding( GKString* pstrEncoding );
	GKbool FileAtTop();
	GKbool FileErrorAddResult();

	FILE* m_fp;
	GKint32 m_nDocFlags;
	GKint32 m_nOpFileByteLen;
	GKint32 m_nBlockSizeBasis;
	MCD_INTFILEOFFSET m_nFileByteLen;
	MCD_INTFILEOFFSET m_nFileByteOffset;
	GKint32 m_nFileCharUnitSize;
	GKint32 m_nOpFileTextLen;
	GKString m_strIOResult;
	GKString m_strEncoding;
	GKString* m_pstrBuffer;
	ElemStack m_elemstack;
	GKint32 m_nReadBufferStart;
	GKint32 m_nReadBufferRemoved;
	GKint32 m_nReadGatherStart;
	GKString m_strReadGatherMarkup;
};

struct BomTableStruct { const char* pszBom; GKint32 nBomLen; GKPCSTR pszBomEnc; GKint32 nBomFlag; } BomTable[] =
{
	{ "\xef\xbb\xbf", 3, _U("UTF-8"), CMarkup::MDF_UTF8PREAMBLE },
	{ "\xff\xfe", 2, _U("UTF-16LE"), CMarkup::MDF_UTF16LEFILE },
	{ "\xfe\xff", 2, _U("UTF-16BE"), CMarkup::MDF_UTF16BEFILE },
	{ NULL,0,NULL,0 }
};

GKbool FilePos::FileErrorAddResult()
{
	// strerror has difficulties cross-platform
	// VC++ leaves GKStringERROR undefined and uses FormatMessage
	// Non-VC++ use strerror (even for MARKUP_WCHAR and convert)
	// additional notes:
	// _WIN32_WCE (Windows CE) has no strerror (Embedded VC++ uses FormatMessage) 
	// _MSC_VER >= 1310 (VC++ 2003/7.1) has _wcserror (but not used)
	//
	const GKint32 nErrorBufferSize = 100;
	GKint32 nErr = 0;
	GKuchar szError[nErrorBufferSize+1];
#if defined(GKStringERROR) // C error routine
	nErr = (GKint32)errno;
#if defined(MARKUP_WCHAR)
	char szMBError[nErrorBufferSize+1];
	strncpy( szMBError, GKStringERROR, nErrorBufferSize );
	szMBError[nErrorBufferSize] = '\0';
	TextEncoding textencoding( _U(""), (const void*)szMBError, strlen(szMBError) );
	textencoding.m_nToCount = nErrorBufferSize;
	GKint32 nWideLen = textencoding.PerformConversion( (void*)szError, MCD_ENC );
	szError[nWideLen] = '\0';
#else
	MCD_PSZNCPY( szError, GKStringERROR, nErrorBufferSize );
	szError[nErrorBufferSize] = '\0';
#endif
#else // no C error routine, use Windows API
	DWORD dwErr = ::GetLastError();
	if ( ::FormatMessage(0x1200,0,dwErr,0,szError,nErrorBufferSize,0) < 1 )
		szError[0] = '\0';
	nErr = (GKint32)dwErr;
#endif // no C error routine
	GKString strError = szError;
	for ( GKint32 nChar=0; nChar<strError.GetLength(); ++nChar )
		if ( strError[nChar] == '\r' || strError[nChar] == '\n' )
		{
			strError = GKStringMID( strError, 0, nChar ); // no trailing newline
			break;
		}
	x_AddResult( m_strIOResult, _U("file_error"), strError, MRC_MSG|MRC_NUMBER, nErr );
	return FALSE;
}

void FilePos::FileSpecifyEncoding( GKString* pstrEncoding )
{
	// In ReadTextFile, WriteTextFile and Open, the pstrEncoding argument can override or return the detected encoding
	if ( pstrEncoding && m_strEncoding != *pstrEncoding )
	{
		if ( m_nFileCharUnitSize == 1 && *pstrEncoding != _U("")  )
			m_strEncoding = *pstrEncoding; // override the encoding
		else // just report the encoding
			*pstrEncoding = m_strEncoding;
	}
}

GKbool FilePos::FileAtTop()
{
	// Return TRUE if in the first block of file mode, max BOM < 5 bytes
	if ( ((m_nDocFlags & CMarkup::MDF_READFILE) && m_nFileByteOffset < (MCD_INTFILEOFFSET)m_nOpFileByteLen + 5 )
			|| ((m_nDocFlags & CMarkup::MDF_WRITEFILE) && m_nFileByteOffset < 5) )
		return TRUE;
	return FALSE;
}

GKbool FilePos::FileOpen( GKPFILENAME szFileName )
{
	GKStringCLEAR( m_strIOResult );

	// Open file
	GKPCSTR_FILENAME pMode = _U_FILENAME("rb");
	if ( m_nDocFlags & CMarkup::MDF_APPENDFILE )
		pMode = _U_FILENAME("ab");
	else if ( m_nDocFlags & CMarkup::MDF_WRITEFILE )
		pMode = _U_FILENAME("wb");
	m_fp = NULL;
	MCD_FOPEN( m_fp, szFileName, pMode );
	if ( ! m_fp )
		return FileErrorAddResult();

	// Prepare file
	GKbool bSuccess = TRUE;
	GKint32 nBomLen = 0;
	m_nFileCharUnitSize = 1; // unless UTF-16 BOM
	if ( m_nDocFlags & CMarkup::MDF_READFILE )
	{
		// Get file length
		MCD_FSEEK( m_fp, 0, SEEK_END );
		m_nFileByteLen = MCD_FTELL( m_fp );
		MCD_FSEEK( m_fp, 0, SEEK_SET );

		// Read the top of the file to check BOM and encoding
		GKint32 nReadTop = 1024;
		if ( m_nFileByteLen < nReadTop )
			nReadTop = (GKint32)m_nFileByteLen;
		if ( nReadTop )
		{
			char* pFileTop = new char[nReadTop];
			if ( nReadTop )
				bSuccess = ( fread( pFileTop, nReadTop, 1, m_fp ) == 1 );
			if ( bSuccess )
			{
				// Check for Byte Order Mark (preamble)
				GKint32 nBomCheck = 0;
				m_nDocFlags &= ~( CMarkup::MDF_UTF16LEFILE | CMarkup::MDF_UTF8PREAMBLE );
				while ( BomTable[nBomCheck].pszBom )
				{
					while ( nBomLen < BomTable[nBomCheck].nBomLen )
					{
						if ( nBomLen >= nReadTop || pFileTop[nBomLen] != BomTable[nBomCheck].pszBom[nBomLen] )
							break;
						++nBomLen;
					}
					if ( nBomLen == BomTable[nBomCheck].nBomLen )
					{
						m_nDocFlags |= BomTable[nBomCheck].nBomFlag;
						if ( nBomLen == 2 )
							m_nFileCharUnitSize = 2;
						m_strEncoding = BomTable[nBomCheck].pszBomEnc;
						break;
					}
					++nBomCheck;
					nBomLen = 0;
				}
				if ( nReadTop > nBomLen )
					MCD_FSEEK( m_fp, nBomLen, SEEK_SET );

				// Encoding check
				if ( ! nBomLen )
				{
					GKString strDeclCheck;
#if defined(MARKUP_WCHAR) // WCHAR
					TextEncoding textencoding( _U("UTF-8"), (const void*)pFileTop, nReadTop );
					GKuchar* pWideBuffer = MCD_GETBUFFER(strDeclCheck,nReadTop);
					textencoding.m_nToCount = nReadTop;
					GKint32 nDeclWideLen = textencoding.PerformConversion( (void*)pWideBuffer, MCD_ENC );
					MCD_RELEASEBUFFER(strDeclCheck,pWideBuffer,nDeclWideLen);
#else // not WCHAR
					GKStringASSIGN(strDeclCheck,pFileTop,nReadTop);
#endif // not WCHAR
					m_strEncoding = CMarkup::GetDeclaredEncoding( strDeclCheck );
				}
				// Assume markup files starting with < sign are UTF-8 if otherwise unknown
				if ( GKStringISEMPTY(m_strEncoding) && pFileTop[0] == '<' )
					m_strEncoding = _U("UTF-8");
			}
			delete [] pFileTop;
		}
	}
	else if ( m_nDocFlags & CMarkup::MDF_WRITEFILE )
	{
		if ( m_nDocFlags & CMarkup::MDF_APPENDFILE )
		{
			// fopen for append does not move the file pointer to the end until first I/O operation
			MCD_FSEEK( m_fp, 0, SEEK_END );
			m_nFileByteLen = MCD_FTELL( m_fp );
		}
		GKint32 nBomCheck = 0;
		while ( BomTable[nBomCheck].pszBom )
		{
			if ( m_nDocFlags & BomTable[nBomCheck].nBomFlag )
			{
				nBomLen = BomTable[nBomCheck].nBomLen;
				if ( nBomLen == 2 )
					m_nFileCharUnitSize = 2;
				m_strEncoding = BomTable[nBomCheck].pszBomEnc;
				if ( m_nFileByteLen ) // append
					nBomLen = 0;
				else // write BOM
					bSuccess = ( fwrite(BomTable[nBomCheck].pszBom,nBomLen,1,m_fp) == 1 );
				break;
			}
			++nBomCheck;
		}
	}
	if ( ! bSuccess )
		return FileErrorAddResult();

	if ( m_nDocFlags & CMarkup::MDF_APPENDFILE )
		m_nFileByteOffset = m_nFileByteLen;
	else
		m_nFileByteOffset = (MCD_INTFILEOFFSET)nBomLen;
	if ( nBomLen )
		x_AddResult( m_strIOResult, _U("bom") );
	return bSuccess;
}

GKbool FilePos::FileRead( void* pBuffer )
{
	GKbool bSuccess = ( fread( pBuffer,m_nOpFileByteLen,1,m_fp) == 1 );
	m_nOpFileTextLen = m_nOpFileByteLen / m_nFileCharUnitSize;
	if ( bSuccess )
	{
		m_nFileByteOffset += m_nOpFileByteLen;
		x_AddResult( m_strIOResult, _U("read"), m_strEncoding, MRC_ENCODING|MRC_LENGTH, m_nOpFileTextLen );

		// Microsoft components can produce apparently valid docs with some nulls at ends of values
		GKint32 nNullCount = 0;
		GKint32 nNullCheckCharsRemaining = m_nOpFileTextLen;
		char* pAfterNull = NULL;
		char* pNullScan = (char*)pBuffer;
		GKbool bSingleByteChar = m_nFileCharUnitSize == 1;
		while ( nNullCheckCharsRemaining-- )
		{
			if ( bSingleByteChar? (! *pNullScan) : (! (*(unsigned short*)pNullScan)) )
			{
				if ( pAfterNull && pNullScan != pAfterNull )
					memmove( pAfterNull - (nNullCount*m_nFileCharUnitSize), pAfterNull, pNullScan - pAfterNull );
				pAfterNull = pNullScan  + m_nFileCharUnitSize;
				++nNullCount;
			}
			pNullScan += m_nFileCharUnitSize;
		}
		if ( pAfterNull && pNullScan != pAfterNull )
			memmove( pAfterNull - (nNullCount*m_nFileCharUnitSize), pAfterNull, pNullScan - pAfterNull );
		if ( nNullCount )
		{
			x_AddResult( m_strIOResult, _U("nulls_removed"), NULL, MRC_COUNT, nNullCount );
			m_nOpFileTextLen -= nNullCount;
		}

		// Big endian/little endian conversion
		if ( m_nFileCharUnitSize > 1 && x_EndianSwapRequired(m_nDocFlags) )
		{
			x_EndianSwapUTF16( (unsigned short*)pBuffer, m_nOpFileTextLen );
			x_AddResult( m_strIOResult, _U("endian_swap") );
		}
	}
	if ( ! bSuccess )
		FileErrorAddResult();
	return bSuccess;
}

GKbool FilePos::FileCheckRaggedEnd( void* pBuffer )
{
	// In file read mode, piece of file text in memory must end on a character boundary
	// This check must happen after the encoding has been decided, so after UTF-8 autodetection
	// If ragged, adjust file position, m_nOpFileTextLen and m_nOpFileByteLen
	GKint32 nTruncBeforeBytes = 0;
	TextEncoding textencoding( m_strEncoding, pBuffer, m_nOpFileTextLen );
	if ( ! textencoding.FindRaggedEnd(nTruncBeforeBytes) )
	{
		// Input must be garbled? decoding error before potentially ragged end, add error result and continue
		GKString strEncoding = m_strEncoding;
		if ( GKStringISEMPTY(strEncoding) )
			strEncoding = _U("ANSI");
		x_AddResult( m_strIOResult, _U("truncation_error"), strEncoding, MRC_ENCODING );
	}
	else if ( nTruncBeforeBytes )
	{
		nTruncBeforeBytes *= -1;
		m_nFileByteOffset += nTruncBeforeBytes;
		MCD_FSEEK( m_fp, m_nFileByteOffset, SEEK_SET );
		m_nOpFileByteLen += nTruncBeforeBytes;
		m_nOpFileTextLen += nTruncBeforeBytes / m_nFileCharUnitSize;
		x_AddResult( m_strIOResult, _U("read"), NULL, MRC_MODIFY|MRC_LENGTH, m_nOpFileTextLen );
	}
	return TRUE;
}

GKbool FilePos::FileReadText( GKString& strDoc )
{
	GKbool bSuccess = TRUE;
	GKStringCLEAR( m_strIOResult );
	if ( ! m_nOpFileByteLen )
	{
		x_AddResult( m_strIOResult, _U("read"), m_strEncoding, MRC_ENCODING|MRC_LENGTH, 0 );
		return bSuccess;
	}

	// Only read up to end of file (a single read byte length cannot be over the capacity of GKint32)
	GKbool bCheckRaggedEnd = TRUE;
	MCD_INTFILEOFFSET nBytesRemaining = m_nFileByteLen - m_nFileByteOffset;
	if ( (MCD_INTFILEOFFSET)m_nOpFileByteLen >= nBytesRemaining )
	{
		m_nOpFileByteLen = (GKint32)nBytesRemaining;
		bCheckRaggedEnd = FALSE;
	}

	if ( m_nDocFlags & (CMarkup::MDF_UTF16LEFILE | CMarkup::MDF_UTF16BEFILE) )
	{
		GKint32 nUTF16Len = m_nOpFileByteLen / 2;
#if defined(MARKUP_WCHAR) // WCHAR
		GKint32 nBufferSizeForGrow = nUTF16Len + nUTF16Len/100; // extra 1%
#if MARKUP_SIZEOFWCHAR == 4 // sizeof(wchar_t) == 4
		unsigned short* pUTF16Buffer = new unsigned short[nUTF16Len+1];
		bSuccess = FileRead( pUTF16Buffer );
		if ( bSuccess )
		{
			if ( bCheckRaggedEnd )
				FileCheckRaggedEnd( (void*)pUTF16Buffer );
			TextEncoding textencoding( _U("UTF-16"), (const void*)pUTF16Buffer, m_nOpFileTextLen );
			textencoding.m_nToCount = nBufferSizeForGrow;
			GKuchar* pUTF32Buffer = MCD_GETBUFFER(strDoc,nBufferSizeForGrow);
			GKint32 nUTF32Len = textencoding.PerformConversion( (void*)pUTF32Buffer, _U("UTF-32") );
			MCD_RELEASEBUFFER(strDoc,pUTF32Buffer,nUTF32Len);
			x_AddResult( m_strIOResult, _U("converted_to"), _U("UTF-32"), MRC_ENCODING|MRC_LENGTH, nUTF32Len );
		}
#else // sizeof(wchar_t) == 2
		GKuchar* pUTF16Buffer = MCD_GETBUFFER(strDoc,nBufferSizeForGrow);
		bSuccess = FileRead( pUTF16Buffer );
		if ( bSuccess && bCheckRaggedEnd )
			FileCheckRaggedEnd( (void*)pUTF16Buffer );
		MCD_RELEASEBUFFER(strDoc,pUTF16Buffer,m_nOpFileTextLen);
#endif // sizeof(wchar_t) == 2
#else // not WCHAR
		// Convert file from UTF-16; it needs to be in memory as UTF-8 or MBCS
		unsigned short* pUTF16Buffer = new unsigned short[nUTF16Len+1];
		bSuccess = FileRead( pUTF16Buffer );
		if ( bSuccess && bCheckRaggedEnd )
			FileCheckRaggedEnd( (void*)pUTF16Buffer );
		TextEncoding textencoding( _U("UTF-16"), (const void*)pUTF16Buffer, m_nOpFileTextLen );
		GKint32 nMBLen = textencoding.PerformConversion( NULL, MCD_ENC );
		GKint32 nBufferSizeForGrow = nMBLen + nMBLen/100; // extra 1%
		GKuchar* pMBBuffer = MCD_GETBUFFER(strDoc,nBufferSizeForGrow);
		textencoding.PerformConversion( (void*)pMBBuffer );
		delete [] pUTF16Buffer;
		MCD_RELEASEBUFFER(strDoc,pMBBuffer,nMBLen);
		x_AddResult( m_strIOResult, _U("converted_to"), MCD_ENC, MRC_ENCODING|MRC_LENGTH, nMBLen );
		if ( textencoding.m_nFailedChars )
			x_AddResult( m_strIOResult, _U("conversion_loss") );
#endif // not WCHAR
	}
	else // single or multibyte file (i.e. not UTF-16)
	{
#if defined(MARKUP_WCHAR) // WCHAR
		char* pBuffer = new char[m_nOpFileByteLen];
		bSuccess = FileRead( pBuffer );
		if ( GKStringISEMPTY(m_strEncoding) )
		{
			GKint32 nNonASCII;
			GKbool bErrorAtEnd;
			if ( CMarkup::DetectUTF8(pBuffer,m_nOpFileByteLen,&nNonASCII,&bErrorAtEnd) || (bCheckRaggedEnd && bErrorAtEnd) )
			{
				m_strEncoding = _U("UTF-8");
				x_AddResult( m_strIOResult, _U("read"), m_strEncoding, MRC_MODIFY|MRC_ENCODING );
			}
			x_AddResult( m_strIOResult, _U("utf8_detection") );
		}
		if ( bSuccess && bCheckRaggedEnd )
			FileCheckRaggedEnd( (void*)pBuffer );
		TextEncoding textencoding( m_strEncoding, (const void*)pBuffer, m_nOpFileTextLen );
		GKint32 nWideLen = textencoding.PerformConversion( NULL, MCD_ENC );
		GKint32 nBufferSizeForGrow = nWideLen + nWideLen/100; // extra 1%
		GKuchar* pWideBuffer = MCD_GETBUFFER(strDoc,nBufferSizeForGrow);
		textencoding.PerformConversion( (void*)pWideBuffer );
		MCD_RELEASEBUFFER( strDoc, pWideBuffer, nWideLen );
		delete [] pBuffer;
		x_AddResult( m_strIOResult, _U("converted_to"), MCD_ENC, MRC_ENCODING|MRC_LENGTH, nWideLen );
#else // not WCHAR
		// After loading a file with unknown multi-byte encoding
		GKbool bAssumeUnknownIsNative = FALSE;
		if ( GKStringISEMPTY(m_strEncoding) )
		{
			bAssumeUnknownIsNative = TRUE;
			m_strEncoding = MCD_ENC;
		}
		if ( TextEncoding::CanConvert(MCD_ENC,m_strEncoding) )
		{
			char* pBuffer = new char[m_nOpFileByteLen];
			bSuccess = FileRead( pBuffer );
			if ( bSuccess && bCheckRaggedEnd )
				FileCheckRaggedEnd( (void*)pBuffer );
			TextEncoding textencoding( m_strEncoding, (const void*)pBuffer, m_nOpFileTextLen );
			GKint32 nMBLen = textencoding.PerformConversion( NULL, MCD_ENC );
			GKint32 nBufferSizeForGrow = nMBLen + nMBLen/100; // extra 1%
			GKuchar* pMBBuffer = MCD_GETBUFFER(strDoc,nBufferSizeForGrow);
			textencoding.PerformConversion( (void*)pMBBuffer );
			MCD_RELEASEBUFFER( strDoc, pMBBuffer, nMBLen );
			delete [] pBuffer;
			x_AddResult( m_strIOResult, _U("converted_to"), MCD_ENC, MRC_ENCODING|MRC_LENGTH, nMBLen );
			if ( textencoding.m_nFailedChars )
				x_AddResult( m_strIOResult, _U("conversion_loss") );
		}
		else // load directly into string
		{
			GKint32 nBufferSizeForGrow = m_nOpFileByteLen + m_nOpFileByteLen/100; // extra 1%
			GKuchar* pBuffer = MCD_GETBUFFER(strDoc,nBufferSizeForGrow);
			bSuccess = FileRead( pBuffer );
			GKbool bConvertMB = FALSE;
			if ( bAssumeUnknownIsNative )
			{
				// Might need additional conversion if we assumed an encoding
				GKint32 nNonASCII;
				GKbool bErrorAtEnd;
				GKbool bIsUTF8 = CMarkup::DetectUTF8( pBuffer, m_nOpFileByteLen, &nNonASCII, &bErrorAtEnd ) || (bCheckRaggedEnd && bErrorAtEnd);
				GKString strDetectedEncoding = bIsUTF8? _U("UTF-8"): _U("");
				if ( nNonASCII && m_strEncoding != strDetectedEncoding ) // only need to convert non-ASCII
					bConvertMB = TRUE;
				m_strEncoding = strDetectedEncoding;
				if ( bIsUTF8 )
					x_AddResult( m_strIOResult, _U("read"), m_strEncoding, MRC_MODIFY|MRC_ENCODING );
			}
			if ( bSuccess && bCheckRaggedEnd )
				FileCheckRaggedEnd( (void*)pBuffer );
			MCD_RELEASEBUFFER( strDoc, pBuffer, m_nOpFileTextLen );
			if ( bConvertMB )
			{
				TextEncoding textencoding( m_strEncoding, MCD_2PCSZ(strDoc), m_nOpFileTextLen );
				GKint32 nMBLen = textencoding.PerformConversion( NULL, MCD_ENC );
				nBufferSizeForGrow = nMBLen + nMBLen/100; // extra 1%
				GKString strConvDoc;
				pBuffer = MCD_GETBUFFER(strConvDoc,nBufferSizeForGrow);
				textencoding.PerformConversion( (void*)pBuffer );
				MCD_RELEASEBUFFER( strConvDoc, pBuffer, nMBLen );
				strDoc = strConvDoc;
				x_AddResult( m_strIOResult, _U("converted_to"), MCD_ENC, MRC_ENCODING|MRC_LENGTH, nMBLen );
				if ( textencoding.m_nFailedChars )
					x_AddResult( m_strIOResult, _U("conversion_loss") );
			}
			if ( bAssumeUnknownIsNative )
				x_AddResult( m_strIOResult, _U("utf8_detection") );
		}
#endif // not WCHAR
	}
	return bSuccess;
}

GKbool FilePos::FileWrite( void* pBuffer, const void* pConstBuffer /*=NULL*/ )
{
	m_nOpFileByteLen = m_nOpFileTextLen * m_nFileCharUnitSize;
	if ( ! pConstBuffer )
		pConstBuffer = pBuffer;
	unsigned short* pTempEndianBuffer = NULL;
	if ( x_EndianSwapRequired(m_nDocFlags) )
	{
		if ( ! pBuffer )
		{
			pTempEndianBuffer = new unsigned short[m_nOpFileTextLen];
			memcpy( pTempEndianBuffer, pConstBuffer, m_nOpFileTextLen * 2 );
			pBuffer = pTempEndianBuffer;
			pConstBuffer = pTempEndianBuffer;
		}
		x_EndianSwapUTF16( (unsigned short*)pBuffer, m_nOpFileTextLen );
		x_AddResult( m_strIOResult, _U("endian_swap") );
	}
	GKbool bSuccess = ( fwrite( pConstBuffer, m_nOpFileByteLen, 1, m_fp ) == 1 );
	if ( pTempEndianBuffer )
		delete [] pTempEndianBuffer;
	if ( bSuccess )
	{
		m_nFileByteOffset += m_nOpFileByteLen;
		x_AddResult( m_strIOResult, _U("write"), m_strEncoding, MRC_ENCODING|MRC_LENGTH, m_nOpFileTextLen );
	}
	else
		FileErrorAddResult();
	return bSuccess;
}

GKbool FilePos::FileWriteText( const GKString& strDoc, GKint32 nWriteStrLen/*=-1*/ )
{
	GKbool bSuccess = TRUE;
	GKStringCLEAR( m_strIOResult );
	GKPCSTR pDoc = MCD_2PCSZ(strDoc);
	if ( nWriteStrLen == -1 )
		nWriteStrLen = strDoc.GetLength();
	if ( ! nWriteStrLen )
	{
		x_AddResult( m_strIOResult, _U("write"), m_strEncoding, MRC_ENCODING|MRC_LENGTH, 0 );
		return bSuccess;
	}

	if ( m_nDocFlags & (CMarkup::MDF_UTF16LEFILE | CMarkup::MDF_UTF16BEFILE) )
	{
#if defined(MARKUP_WCHAR) // WCHAR
#if MARKUP_SIZEOFWCHAR == 4 // sizeof(wchar_t) == 4
		TextEncoding textencoding( _U("UTF-32"), (const void*)pDoc, nWriteStrLen );
		m_nOpFileTextLen = textencoding.PerformConversion( NULL, _U("UTF-16") );
		unsigned short* pUTF16Buffer = new unsigned short[m_nOpFileTextLen];
		textencoding.PerformConversion( (void*)pUTF16Buffer );
		x_AddResult( m_strIOResult, _U("converted_from"), _U("UTF-32"), MRC_ENCODING|MRC_LENGTH, nWriteStrLen );
		bSuccess = FileWrite( pUTF16Buffer );
		delete [] pUTF16Buffer;
#else // sizeof(wchar_t) == 2
		m_nOpFileTextLen = nWriteStrLen;
		bSuccess = FileWrite( NULL, pDoc );
#endif
#else // not WCHAR
		TextEncoding textencoding( MCD_ENC, (const void*)pDoc, nWriteStrLen );
		m_nOpFileTextLen = textencoding.PerformConversion( NULL, _U("UTF-16") );
		unsigned short* pUTF16Buffer = new unsigned short[m_nOpFileTextLen];
		textencoding.PerformConversion( (void*)pUTF16Buffer );
		x_AddResult( m_strIOResult, _U("converted_from"), MCD_ENC, MRC_ENCODING|MRC_LENGTH, nWriteStrLen );
		bSuccess = FileWrite( pUTF16Buffer );
		delete [] pUTF16Buffer;
#endif // not WCHAR
	}
	else // single or multibyte file (i.e. not UTF-16)
	{
#if ! defined(MARKUP_WCHAR) // not WCHAR
		if ( ! TextEncoding::CanConvert(m_strEncoding,MCD_ENC) )
		{
			// Same or unsupported multi-byte to multi-byte, so save directly from string
			m_nOpFileTextLen = nWriteStrLen;
			bSuccess = FileWrite( NULL, pDoc );
			return bSuccess;
		}
#endif // not WCHAR
		TextEncoding textencoding( MCD_ENC, (const void*)pDoc, nWriteStrLen );
		m_nOpFileTextLen = textencoding.PerformConversion( NULL, m_strEncoding );
		char* pMBBuffer = new char[m_nOpFileTextLen];
		textencoding.PerformConversion( (void*)pMBBuffer );
		x_AddResult( m_strIOResult, _U("converted_from"), MCD_ENC, MRC_ENCODING|MRC_LENGTH, nWriteStrLen );
		if ( textencoding.m_nFailedChars )
			x_AddResult( m_strIOResult, _U("conversion_loss") );
		bSuccess = FileWrite( pMBBuffer );
		delete [] pMBBuffer;
	}

	return bSuccess;
}

GKbool FilePos::FileClose()
{
	if ( m_fp )
	{
		if ( fclose(m_fp) )
			FileErrorAddResult();
		m_fp = NULL;
		m_nDocFlags &= ~(CMarkup::MDF_WRITEFILE|CMarkup::MDF_READFILE|CMarkup::MDF_APPENDFILE);
		return TRUE;
	}
	return FALSE;
}

GKbool FilePos::FileReadNextBuffer()
{
	// If not end of file, returns amount to subtract from offsets
	if ( m_nFileByteOffset < m_nFileByteLen )
	{
		// Prepare to put this node at beginning
		GKString& str = *m_pstrBuffer;
		GKint32 nDocLength = str .GetLength();
		GKint32 nRemove = m_nReadBufferStart;
		m_nReadBufferRemoved = nRemove;

		// Gather
		if ( m_nReadGatherStart != -1 )
		{
			if ( m_nReadBufferStart > m_nReadGatherStart )
			{
				// In case it is a large subdoc, reduce reallocs by using x_StrInsertReplace
				GKString strAppend = GKStringMID( str, m_nReadGatherStart, m_nReadBufferStart - m_nReadGatherStart );
				x_StrInsertReplace( m_strReadGatherMarkup, m_strReadGatherMarkup.GetLength(), 0, strAppend );
			}
			m_nReadGatherStart = 0;
		}

		// Increase capacity if keeping more than half of nDocLength
		GKint32 nKeepLength = nDocLength - nRemove;
		if ( nKeepLength > nDocLength / 2 )
			m_nBlockSizeBasis *= 2;
		if ( nRemove )
			x_StrInsertReplace( str, 0, nRemove, GKString() );
		GKString strRead;
		m_nOpFileByteLen = m_nBlockSizeBasis - nKeepLength;
		m_nOpFileByteLen += 4 - m_nOpFileByteLen % 4; // round up to 4-byte offset
		FileReadText( strRead );
		x_StrInsertReplace( str, nKeepLength, 0, strRead );
		m_nReadBufferStart = 0; // next time just elongate/increase capacity
		return TRUE;
	}
	return FALSE;
}

void FilePos::FileGatherStart( GKint32 nStart )
{
	m_nReadGatherStart = nStart;
}

GKint32 FilePos::FileGatherEnd( GKString& strMarkup )
{
	GKint32 nStart = m_nReadGatherStart;
	m_nReadGatherStart = -1;
	strMarkup = m_strReadGatherMarkup;
	GKStringCLEAR( m_strReadGatherMarkup );
	return nStart;
}

GKbool FilePos::FileFlush( GKString& strBuffer, GKint32 nWriteStrLen/*=-1*/, GKbool bFflush/*=FALSE*/ )
{
	GKbool bSuccess = TRUE;
	GKStringCLEAR( m_strIOResult );
	if ( nWriteStrLen == -1 )
		nWriteStrLen = strBuffer .GetLength();
	if ( nWriteStrLen )
	{
		if ( (! m_nFileByteOffset) && GKStringISEMPTY(m_strEncoding) && ! GKStringISEMPTY(strBuffer) )
		{
			m_strEncoding = CMarkup::GetDeclaredEncoding( strBuffer );
			if ( GKStringISEMPTY(m_strEncoding) )
				m_strEncoding = _U("UTF-8");
		}
		bSuccess = FileWriteText( strBuffer, nWriteStrLen );
		if ( bSuccess )
			x_StrInsertReplace( strBuffer, 0, nWriteStrLen, GKString() );
	}
	if ( bFflush && bSuccess )
	{
		if ( fflush(m_fp) )
			bSuccess = FileErrorAddResult();
	}
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// PathPos encapsulates parsing of the path string used in Find methods
//
struct PathPos
{
	PathPos( GKPCSTR pszPath, GKbool b ) { p=pszPath; bReader=b; i=0; iPathAttribName=0; iSave=0; nPathType=0; if (!ParsePath()) nPathType=-1; };
	GKint32 GetTypeAndInc() { i=-1; if (p) { if (p[0]=='/') { if (p[1]=='/') i=2; else i=1; } else if (p[0]) i=0; } nPathType=i+1; return nPathType; };
	GKint32 GetNumAndInc() { GKint32 n=0; while (p[i]>='0'&&p[i]<='9') n=n*10+(GKint32)p[i++]-(GKint32)'0'; return n; };
	GKPCSTR GetValAndInc() { ++i; GKuchar cEnd=']'; if (p[i]=='\''||p[i]=='\"') cEnd=p[i++]; GKint32 iVal=i; IncWord(cEnd); nLen=i-iVal; if (cEnd!=']') ++i; return &p[iVal]; };
	GKint32 GetValOrWordLen() { return nLen; };
	GKuchar GetChar() { return p[i]; };
	GKbool IsAtPathEnd() { return ((!p[i])||(iPathAttribName&&i+2>=iPathAttribName))?TRUE:FALSE; }; 
	GKPCSTR GetPtr() { return &p[i]; };
	void SaveOffset() { iSave=i; };
	void RevertOffset() { i=iSave; };
	void RevertOffsetAsName() { i=iSave; nPathType=1; };
	GKPCSTR GetWordAndInc() { GKint32 iWord=i; IncWord(); nLen=i-iWord; return &p[iWord]; };
	void IncWord() { while (p[i]&&!x_ISENDPATHWORD(p[i])) i+=MCD_CLEN(&p[i]); };
	void IncWord( GKuchar c ) { while (p[i]&&p[i]!=c) i+=MCD_CLEN(&p[i]); };
	void IncChar() { ++i; };
	void Inc( GKint32 n ) { i+=n; };
	GKbool IsAnywherePath() { return nPathType == 3; };
	GKbool IsAbsolutePath() { return nPathType == 2; };
	GKbool IsPath() { return nPathType > 0; };
	GKbool ValidPath() { return nPathType != -1; };
	GKPCSTR GetPathAttribName() { if (iPathAttribName) return &p[iPathAttribName]; return NULL; };
	GKbool AttribPredicateMatch( TokenPos& token );
private:
	GKbool ParsePath();
	GKint32 nPathType; // -1 invalid, 0 empty, 1 name, 2 absolute path, 3 anywhere path
	GKbool bReader;
	GKPCSTR p;
	GKint32 i;
	GKint32 iPathAttribName;
	GKint32 iSave;
	GKint32 nLen;
};

GKbool PathPos::ParsePath()
{
	// Determine if the path seems to be in a valid format before attempting to find
	if ( GetTypeAndInc() )
	{
		SaveOffset();
		while ( 1 )
		{
			if ( ! GetChar() )
				return FALSE;
			IncWord(); // Tag name
			if ( GetChar() == '[' ) // predicate
			{
				IncChar(); // [
				if ( GetChar() >= '1' && GetChar() <= '9' )
					GetNumAndInc();
				else // attrib or child tag name
				{
					if ( GetChar() == '@' )
					{
						IncChar(); // @
						IncWord(); // attrib name
						if ( GetChar() == '=' )
							GetValAndInc();
					}
					else
					{
						if ( bReader )
							return FALSE;
						IncWord();
					}
				}
				if ( GetChar() != ']' )
					return FALSE;
				IncChar(); // ]
			}

			// Another level of path
			if ( GetChar() == '/' )
			{
				if ( IsAnywherePath() )
					return FALSE; // multiple levels not supported for // path
				IncChar();
				if ( GetChar() == '@' )
				{
					// FindGetData and FindSetData support paths ending in attribute
					IncChar(); // @
					iPathAttribName = i;
					IncWord(); // attrib name
					if ( GetChar() )
						return FALSE; // it should have ended with attribute name
					break;
				}
			}
			else
			{
				if ( GetChar() )
					return FALSE; // not a slash, so it should have ended here
				break;
			}
		}
		RevertOffset();
	}
	return TRUE;
}

GKbool PathPos::AttribPredicateMatch( TokenPos& token )
{
	// Support attribute predicate matching in regular and file read mode
	// token.m_nNext must already be set to node.nStart + 1 or ELEM(i).nStart + 1
	IncChar(); // @
	if ( token.FindAttrib(GetPtr()) )
	{
		IncWord();
		if ( GetChar() == '=' )
		{
			GKPCSTR pszVal = GetValAndInc();
			GKString strPathValue = CMarkup::UnescapeText( pszVal, GetValOrWordLen() );
			GKString strAttribValue = CMarkup::UnescapeText( token.GetTokenPtr(), token.Length(), token.m_nTokenFlags );
			if ( strPathValue != strAttribValue )
				return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// A map is a table of SavedPos structs
//
struct SavedPos
{
	// SavedPos is an entry in the SavedPosMap hash table
	SavedPos() { nSavedPosFlags=0; iPos=0; };
	GKString strName;
	GKint32 iPos;
	enum { SPM_MAIN = 1, SPM_CHILD = 2, SPM_USED = 4, SPM_LAST = 8 };
	GKint32 nSavedPosFlags;
};

struct SavedPosMap
{
	// SavedPosMap is only created if SavePos/RestorePos are used
	SavedPosMap( GKint32 nSize ) { nMapSize=nSize; pTable = new SavedPos*[nSize]; memset(pTable,0,nSize*sizeof(SavedPos*)); };
	~SavedPosMap() { if (pTable) { for (GKint32 n=0;n<nMapSize;++n) if (pTable[n]) delete[] pTable[n]; delete[] pTable; } };
	SavedPos** pTable;
	GKint32 nMapSize;
};

struct SavedPosMapArray
{
	// SavedPosMapArray keeps pointers to SavedPosMap instances
	SavedPosMapArray() { m_pMaps = NULL; };
	~SavedPosMapArray() { ReleaseMaps(); };
	void ReleaseMaps() { SavedPosMap**p = m_pMaps; if (p) { while (*p) delete *p++; delete[] m_pMaps; m_pMaps=NULL; } };
	GKbool GetMap( SavedPosMap*& pMap, GKint32 nMap, GKint32 nMapSize = 7 );
	void CopySavedPosMaps( SavedPosMapArray* pOtherMaps );
	SavedPosMap** m_pMaps; // NULL terminated array
};

GKbool SavedPosMapArray::GetMap( SavedPosMap*& pMap, GKint32 nMap, GKint32 nMapSize /*=7*/ )
{
	// Find or create map, returns TRUE if map(s) created
	SavedPosMap** pMapsExisting = m_pMaps;
	GKint32 nMapIndex = 0;
	if ( pMapsExisting )
	{
		// Length of array is unknown, so loop through maps
		while ( nMapIndex <= nMap )
		{
			pMap = pMapsExisting[nMapIndex];
			if ( ! pMap )
				break;
			if ( nMapIndex == nMap )
				return FALSE; // not created
			++nMapIndex;
		}
		nMapIndex = 0;
	}

	// Create map(s)
	// If you access map 1 before map 0 created, then 2 maps will be created
	m_pMaps = new SavedPosMap*[nMap+2];
	if ( pMapsExisting )
	{
		while ( pMapsExisting[nMapIndex] )
		{
			m_pMaps[nMapIndex] = pMapsExisting[nMapIndex];
			++nMapIndex;
		}
		delete[] pMapsExisting;
	}
	while ( nMapIndex <= nMap )
	{
		m_pMaps[nMapIndex] = new SavedPosMap( nMapSize );
		++nMapIndex;
	}
	m_pMaps[nMapIndex] = NULL;
	pMap = m_pMaps[nMap];
	return TRUE; // map(s) created
}

void SavedPosMapArray::CopySavedPosMaps( SavedPosMapArray* pOtherMaps )
{
	ReleaseMaps();
	if ( pOtherMaps->m_pMaps )
	{
		GKint32 nMap = 0;
		SavedPosMap* pMap = NULL;
		while ( pOtherMaps->m_pMaps[nMap] )
		{
			SavedPosMap* pMapSrc = pOtherMaps->m_pMaps[nMap];
			GetMap( pMap, nMap, pMapSrc->nMapSize );
			for ( GKint32 nSlot=0; nSlot < pMap->nMapSize; ++nSlot )
			{
				SavedPos* pCopySavedPos = pMapSrc->pTable[nSlot];
				if ( pCopySavedPos )
				{
					GKint32 nCount = 0;
					while ( pCopySavedPos[nCount].nSavedPosFlags & SavedPos::SPM_USED )
					{
						++nCount;
						if ( pCopySavedPos[nCount-1].nSavedPosFlags & SavedPos::SPM_LAST )
							break;
					}
					if ( nCount )
					{
						SavedPos* pNewSavedPos = new SavedPos[nCount];
						for ( GKint32 nCopy=0; nCopy<nCount; ++nCopy )
							pNewSavedPos[nCopy] = pCopySavedPos[nCopy];
						pNewSavedPos[nCount-1].nSavedPosFlags |= SavedPos::SPM_LAST;
						pMap->pTable[nSlot] = pNewSavedPos;
					}
				}
			}
			++nMap;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Core parser function
//
GKint32 TokenPos::ParseNode( NodePos& node )
{
	// Call this with m_nNext set to the start of the node or tag
	// Upon return m_nNext points to the char after the node or tag
	// m_nL and m_nR are set to name location if it is a tag with a name
	// node members set to node location, strMeta used for parse error
	// 
	// <!--...--> comment
	// <!DOCTYPE ...> dtd
	// <?target ...?> processing instruction
	// <![CDATA[...]]> cdata section
	// <NAME ...> element start tag
	// </NAME ...> element end tag
	//
	// returns the nodetype or
	// 0 for end tag
	// -1 for bad node
	// -2 for end of document
	//
	enum ParseBits
	{
		PD_OPENTAG = 1,
		PD_BANG = 2,
		PD_DASH = 4,
		PD_BRACKET = 8,
		PD_TEXTORWS = 16,
		PD_DOCTYPE = 32,
		PD_INQUOTE_S = 64,
		PD_INQUOTE_D = 128,
		PD_EQUALS = 256,
		PD_NOQUOTEVAL = 512
	};
	GKint32 nParseFlags = 0;

	GKPCSTR pFindEnd = NULL;
	GKint32 nNodeType = -1;
	GKint32 nEndLen = 0;
	GKint32 nName = 0;
	GKint32 nNameLen = 0;
	unsigned GKint32 cDminus1 = 0, cDminus2 = 0;
	#define FINDNODETYPE(e,t) { pFindEnd=e; nEndLen=(sizeof(e)-1)/sizeof(GKuchar); nNodeType=t; }
	#define FINDNODETYPENAME(e,t,n) { FINDNODETYPE(e,t) nName=(GKint32)(pD-m_pDocText)+n; }
	#define FINDNODEBAD(e) { pFindEnd=_U(">"); nEndLen=1; x_AddResult(node.strMeta,e,NULL,0,m_nNext); nNodeType=-1; }

	node.nStart = m_nNext;
	node.nNodeFlags = 0;

	GKPCSTR pD = &m_pDocText[m_nNext];
	unsigned GKint32 cD;
	while ( 1 )
	{
		cD = (unsigned GKint32)*pD;
		if ( ! cD )
		{
			m_nNext = (GKint32)(pD - m_pDocText);
			if ( m_pReaderFilePos ) // read file mode
			{
				// Read buffer may only be removed on the first FileReadNextBuffer in this node
				GKint32 nRemovedAlready = m_pReaderFilePos->m_nReadBufferRemoved;
				if ( m_pReaderFilePos->FileReadNextBuffer() ) // more text in file?
				{
					GKint32 nNodeLength = m_nNext - node.nStart;
					GKint32 nRemove = m_pReaderFilePos->m_nReadBufferRemoved;
					if ( nRemove )
					{
						node.nStart -= nRemove;
						if ( nName )
							nName -= nRemove;
						else if ( nNameLen )
						{
							m_nL -= nRemove;
							m_nR -= nRemove;
						}
						m_nNext -= nRemove;
					}
					GKint32 nNewOffset = node.nStart + nNodeLength;
					GKString& str = *m_pReaderFilePos->m_pstrBuffer;
					m_pDocText = MCD_2PCSZ( str );
					pD = &m_pDocText[nNewOffset];
					cD = (unsigned GKint32)*pD; // loaded char replaces null terminator
				}
				if (nRemovedAlready) // preserve m_nReadBufferRemoved for caller of ParseNode
					m_pReaderFilePos->m_nReadBufferRemoved = nRemovedAlready;
			}
			if ( ! cD )
			{
				if ( m_nNext == node.nStart )
				{
					node.nLength = 0;
					node.nNodeType = 0;
					return -2; // end of document
				}
				if ( nNodeType != CMarkup::MNT_WHITESPACE && nNodeType != CMarkup::MNT_TEXT )
				{
					GKPCSTR pType = _U("tag");
					if ( (nParseFlags & PD_DOCTYPE) || nNodeType == CMarkup::MNT_DOCUMENT_TYPE )
						pType = _U("document_type");
					else if ( nNodeType == CMarkup::MNT_ELEMENT )
						pType = _U("start_tag");
					else if ( nNodeType == 0 )
						pType = _U("end_tag");
					else if ( nNodeType == CMarkup::MNT_CDATA_SECTION )
						pType = _U("cdata_section");
					else if ( nNodeType == CMarkup::MNT_PROCESSING_INSTRUCTION )
						pType = _U("processing_instruction");
					else if ( nNodeType == CMarkup::MNT_COMMENT )
						pType = _U("comment");
					nNodeType = -1;
					x_AddResult(node.strMeta,_U("unterminated_tag_syntax"),pType,MRC_TYPE,node.nStart);
				}
				break;
			}
		}

		if ( nName )
		{
			if ( x_ISENDNAME(cD) )
			{
				nNameLen = (GKint32)(pD - m_pDocText) - nName;
				m_nL = nName;
				m_nR = nName + nNameLen - 1;
				nName = 0;
				cDminus2 = 0;
				cDminus1 = 0;
			}
			else
			{
				pD += MCD_CLEN( pD );
				continue;
			}
		}

		if ( pFindEnd )
		{
			if ( cD == '>' && ! (nParseFlags & (PD_INQUOTE_S|PD_INQUOTE_D)) )
			{
				m_nNext = (GKint32)(pD - m_pDocText) + 1;
				if ( nEndLen == 1 )
				{
					pFindEnd = NULL;
					if ( nNodeType == CMarkup::MNT_ELEMENT && cDminus1 == '/' )
					{
						if ( (! cDminus2) || (!(nParseFlags&PD_NOQUOTEVAL)) || x_ISNOTSECONDLASTINVAL(cDminus2) )
							node.nNodeFlags |= MNF_EMPTY;
					}
				}
				else if ( m_nNext - 1 > nEndLen )
				{
					// Test for end of PI or comment
					GKPCSTR pEnd = pD - nEndLen + 1;
					GKPCSTR pInFindEnd = pFindEnd;
					GKint32 nLen = nEndLen;
					while ( --nLen && *pEnd++ == *pInFindEnd++ );
					if ( nLen == 0 )
						pFindEnd = NULL;
				}
				nParseFlags &= ~PD_NOQUOTEVAL; // make sure PD_NOQUOTEVAL is off
				if ( ! pFindEnd && ! (nParseFlags & PD_DOCTYPE) )
					break;
			}
			else if ( cD == '<' && (nNodeType == CMarkup::MNT_TEXT || nNodeType == -1) )
			{
				m_nNext = (GKint32)(pD - m_pDocText);
				break;
			}
			else if ( nNodeType & CMarkup::MNT_ELEMENT )
			{
				if ( (nParseFlags & (PD_INQUOTE_S|PD_INQUOTE_D|PD_NOQUOTEVAL)) )
				{
					if ( cD == '\"' && (nParseFlags&PD_INQUOTE_D) )
						nParseFlags ^= PD_INQUOTE_D; // off
					else if ( cD == '\'' && (nParseFlags&PD_INQUOTE_S) )
						nParseFlags ^= PD_INQUOTE_S; // off
					else if ( (nParseFlags&PD_NOQUOTEVAL) && x_ISWHITESPACE(cD) )
						nParseFlags ^= PD_NOQUOTEVAL; // off
				}
				else // not in attrib value
				{
					// Only set INQUOTE status when preceeded by equal sign
					if ( cD == '\"' && (nParseFlags&PD_EQUALS) )
						nParseFlags ^= PD_INQUOTE_D|PD_EQUALS; // D on, equals off
					else if ( cD == '\'' && (nParseFlags&PD_EQUALS) )
						nParseFlags ^= PD_INQUOTE_S|PD_EQUALS; // S on, equals off
					else if ( cD == '=' && cDminus1 != '=' && ! (nParseFlags&PD_EQUALS) )
						nParseFlags ^= PD_EQUALS; // on
					else if ( (nParseFlags&PD_EQUALS) && ! x_ISWHITESPACE(cD) )
						nParseFlags ^= PD_NOQUOTEVAL|PD_EQUALS; // no quote val on, equals off
				}
				cDminus2 = cDminus1;
				cDminus1 = cD;
			}
			else if ( nNodeType & CMarkup::MNT_DOCUMENT_TYPE )
			{
				if ( cD == '\"' && ! (nParseFlags&PD_INQUOTE_S) )
					nParseFlags ^= PD_INQUOTE_D; // toggle
				else if ( cD == '\'' && ! (nParseFlags&PD_INQUOTE_D) )
					nParseFlags ^= PD_INQUOTE_S; // toggle
			}
		}
		else if ( nParseFlags )
		{
			if ( nParseFlags & PD_TEXTORWS )
			{
				if ( cD == '<' )
				{
					m_nNext = (GKint32)(pD - m_pDocText);
					nNodeType = CMarkup::MNT_WHITESPACE;
					break;
				}
				else if ( ! x_ISWHITESPACE(cD) )
				{
					nParseFlags ^= PD_TEXTORWS;
					FINDNODETYPE( _U("<"), CMarkup::MNT_TEXT )
				}
			}
			else if ( nParseFlags & PD_OPENTAG )
			{
				nParseFlags ^= PD_OPENTAG;
				if ( cD > 0x60 || ( cD > 0x40 && cD < 0x5b ) || cD == 0x5f || cD == 0x3a )
					FINDNODETYPENAME( _U(">"), CMarkup::MNT_ELEMENT, 0 )
				else if ( cD == '/' )
					FINDNODETYPENAME( _U(">"), 0, 1 )
				else if ( cD == '!' )
					nParseFlags |= PD_BANG;
				else if ( cD == '?' )
					FINDNODETYPENAME( _U("?>"), CMarkup::MNT_PROCESSING_INSTRUCTION, 1 )
				else
					FINDNODEBAD( _U("first_tag_syntax") )
			}
			else if ( nParseFlags & PD_BANG )
			{
				nParseFlags ^= PD_BANG;
				if ( cD == '-' )
					nParseFlags |= PD_DASH;
				else if ( nParseFlags & PD_DOCTYPE )
				{
					if ( x_ISDOCTYPESTART(cD) ) // <!ELEMENT ATTLIST ENTITY NOTATION
						FINDNODETYPE( _U(">"), CMarkup::MNT_DOCUMENT_TYPE )
					else
						FINDNODEBAD( _U("doctype_tag_syntax") )
				}
				else
				{
					if ( cD == '[' )
						nParseFlags |= PD_BRACKET;
					else if ( cD == 'D' )
						nParseFlags |= PD_DOCTYPE;
					else
						FINDNODEBAD( _U("exclamation_tag_syntax") )
				}
			}
			else if ( nParseFlags & PD_DASH )
			{
				nParseFlags ^= PD_DASH;
				if ( cD == '-' )
					FINDNODETYPE( _U("-->"), CMarkup::MNT_COMMENT )
				else
					FINDNODEBAD( _U("comment_tag_syntax") )
			}
			else if ( nParseFlags & PD_BRACKET )
			{
				nParseFlags ^= PD_BRACKET;
				if ( cD == 'C' )
					FINDNODETYPE( _U("]]>"), CMarkup::MNT_CDATA_SECTION )
				else
					FINDNODEBAD( _U("cdata_section_syntax") )
			}
			else if ( nParseFlags & PD_DOCTYPE )
			{
				if ( cD == '<' )
					nParseFlags |= PD_OPENTAG;
				else if ( cD == '>' )
				{
					m_nNext = (GKint32)(pD - m_pDocText) + 1;
					nNodeType = CMarkup::MNT_DOCUMENT_TYPE;
					break;
				}
			}
		}
		else if ( cD == '<' )
		{
			nParseFlags |= PD_OPENTAG;
		}
		else
		{
			nNodeType = CMarkup::MNT_WHITESPACE;
			if ( x_ISWHITESPACE(cD) )
				nParseFlags |= PD_TEXTORWS;
			else
				FINDNODETYPE( _U("<"), CMarkup::MNT_TEXT )
		}
		pD += MCD_CLEN( pD );
	}
	node.nLength = m_nNext - node.nStart;
	node.nNodeType = nNodeType;
	return nNodeType;
}

//////////////////////////////////////////////////////////////////////
// CMarkup public methods
//
CMarkup::~CMarkup()
{
	delete m_pSavedPosMaps;
	delete m_pElemPosTree;
}

void CMarkup::operator=( const CMarkup& markup )
{
	// Copying not supported during file mode because of file pointer
	if ( (m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE)) || (markup.m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE)) )
		return;
	m_iPosParent = markup.m_iPosParent;
	m_iPos = markup.m_iPos;
	m_iPosChild = markup.m_iPosChild;
	m_iPosFree = markup.m_iPosFree;
	m_iPosDeleted = markup.m_iPosDeleted;
	m_nNodeType = markup.m_nNodeType;
	m_nNodeOffset = markup.m_nNodeOffset;
	m_nNodeLength = markup.m_nNodeLength;
	m_strDoc = markup.m_strDoc;
	m_strResult = markup.m_strResult;
	m_nDocFlags = markup.m_nDocFlags;
	m_pElemPosTree->CopyElemPosTree( markup.m_pElemPosTree, m_iPosFree );
	m_pSavedPosMaps->CopySavedPosMaps( markup.m_pSavedPosMaps );
	MARKUP_SETDEBUGSTATE;
}

GKbool CMarkup::SetDoc( GKPCSTR pDoc )
{
	// pDoc is markup text, not a filename!
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	// Set document text
	if ( pDoc )
		m_strDoc = pDoc;
	else
	{
		m_strDoc.Empty();
		m_pElemPosTree->ReleaseElemPosTree();
	}

	m_strResult.Empty();
	return x_ParseDoc();
}

GKbool CMarkup::SetDoc( const GKString& strDoc )
{
	// strDoc is markup text, not a filename!
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	m_strDoc = strDoc;
	m_strResult.Empty();
	return x_ParseDoc();
}

GKbool CMarkup::IsWellFormed()
{
	if ( m_nDocFlags & MDF_WRITEFILE )
		return TRUE;
	if ( m_nDocFlags & MDF_READFILE )
	{
		if ( ! (ELEM(0).nFlags & MNF_ILLFORMED) )
			return TRUE;
	}
	else if ( m_pElemPosTree->GetSize()
			&& ! (ELEM(0).nFlags & MNF_ILLFORMED)
			&& ELEM(0).iElemChild
			&& ! ELEM(ELEM(0).iElemChild).iElemNext )
		return TRUE;
	return FALSE;
}

GKString CMarkup::GetError() const
{
	// For backwards compatibility, return a readable English string built from m_strResult
	// In release 11.0 you can use GetResult and examine result in XML format
	CMarkup mResult( m_strResult );
	GKString strError;
	GKint32 nSyntaxErrors = 0;
	while ( mResult.FindElem() )
	{
		GKString strItem;
		GKString strID = mResult.GetTagName();

		// Parse result
		if ( strID == _U("root_has_sibling") )
			strItem = _U("root element has sibling");
		else if ( strID == _U("no_root_element") )
			strItem = _U("no root element");
		else if ( strID == _U("lone_end_tag") )
			strItem = _U("lone end tag '") + mResult.GetAttrib(_U("tagname")) + _U("' at offset ")
				+ mResult.GetAttrib(_U("offset"));
		else if ( strID == _U("unended_start_tag") )
			strItem = _U("start tag '") + mResult.GetAttrib(_U("tagname")) + _U("' at offset ")
				+ mResult.GetAttrib(_U("offset")) + _U(" expecting end tag at offset ") + mResult.GetAttrib(_U("offset2"));
		else if ( strID == _U("first_tag_syntax") )
			strItem = _U("tag syntax error at offset ") + mResult.GetAttrib(_U("offset"))
				+ _U(" expecting tag name / ! or ?");
		else if ( strID == _U("exclamation_tag_syntax") )
			strItem = _U("tag syntax error at offset ") + mResult.GetAttrib(_U("offset"))
				+ _U(" expecting 'DOCTYPE' [ or -");
		else if ( strID == _U("doctype_tag_syntax") )
			strItem = _U("tag syntax error at offset ") + mResult.GetAttrib(_U("offset"))
				+ _U(" expecting markup declaration"); // ELEMENT ATTLIST ENTITY NOTATION
		else if ( strID == _U("comment_tag_syntax") )
			strItem = _U("tag syntax error at offset ") + mResult.GetAttrib(_U("offset"))
				+ _U(" expecting - to begin comment");
		else if ( strID == _U("cdata_section_syntax") )
			strItem = _U("tag syntax error at offset ") + mResult.GetAttrib(_U("offset"))
				+ _U(" expecting 'CDATA'");
		else if ( strID == _U("unterminated_tag_syntax") )
			strItem = _U("unterminated tag at offset ") + mResult.GetAttrib(_U("offset"));

		// Report only the first syntax or well-formedness error
		if ( ! GKStringISEMPTY(strItem) )
		{
			++nSyntaxErrors;
			if ( nSyntaxErrors > 1 )
				continue;
		}

		// I/O results
		if ( strID == _U("file_error") )
			strItem = mResult.GetAttrib(_U("msg"));
		else if ( strID == _U("bom") )
			strItem = _U("BOM +");
		else if ( strID == _U("read") || strID == _U("write") || strID == _U("converted_to") || strID == _U("converted_from") )
		{
			if ( strID == _U("converted_to") )
				strItem = _U("to ");
			GKString strEncoding = mResult.GetAttrib( _U("encoding") );
			if ( ! GKStringISEMPTY(strEncoding) )
				strItem += strEncoding + _U(" ");
			strItem += _U("length ") + mResult.GetAttrib(_U("length"));
			if ( strID == _U("converted_from") )
				strItem += _U(" to");
		}
		else if ( strID == _U("nulls_removed") )
			strItem = _U("removed ") + mResult.GetAttrib(_U("count")) + _U(" nulls");
		else if ( strID == _U("conversion_loss") )
			strItem = _U("(chars lost in conversion!)");
		else if ( strID == _U("utf8_detection") )
			strItem = _U("(used UTF-8 detection)");
		else if ( strID == _U("endian_swap") )
			strItem = _U("endian swap");
		else if ( strID == _U("truncation_error") )
			strItem = _U("encoding ") + mResult.GetAttrib(_U("encoding")) + _U(" adjustment error");

		// Concatenate result item to error string
		if ( ! GKStringISEMPTY(strItem) )
		{
			if ( ! GKStringISEMPTY(strError) )
				strError += _U(" ");
			strError += strItem;
		}
	}
	return strError;
}

GKbool CMarkup::Load( GKPFILENAME szFileName )
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	if ( ! ReadTextFile(szFileName, m_strDoc, &m_strResult, &m_nDocFlags) )
		return FALSE;
	return x_ParseDoc();
}

GKbool CMarkup::ReadTextFile( GKPFILENAME szFileName, GKString& strDoc, GKString* pstrResult, GKint32* pnDocFlags, GKString* pstrEncoding )
{
	// Static utility method to load text file into strDoc
	//
	FilePos file;
	file.m_nDocFlags = (pnDocFlags?*pnDocFlags:0) | MDF_READFILE;
	GKbool bSuccess = file.FileOpen( szFileName );
	if ( pstrResult )
		*pstrResult = file.m_strIOResult;
	strDoc.Empth();
	if ( bSuccess )
	{
		file.FileSpecifyEncoding( pstrEncoding );
		file.m_nOpFileByteLen = (GKint32)((MCD_INTFILEOFFSET)(file.m_nFileByteLen - file.m_nFileByteOffset));
		bSuccess = file.FileReadText( strDoc );
		file.FileClose();
		if ( pstrResult )
			*pstrResult += file.m_strIOResult;
		if ( pnDocFlags )
			*pnDocFlags = file.m_nDocFlags;
	}
	return bSuccess;
}

GKbool CMarkup::Save( GKPFILENAME szFileName )
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	return WriteTextFile( szFileName, m_strDoc, &m_strResult, &m_nDocFlags );
}

GKbool CMarkup::WriteTextFile( GKPFILENAME szFileName, const GKString& strDoc, GKString* pstrResult, GKint32* pnDocFlags, GKString* pstrEncoding )
{
	// Static utility method to save strDoc to text file
	//
	FilePos file;
	file.m_nDocFlags = (pnDocFlags?*pnDocFlags:0) | MDF_WRITEFILE;
	GKbool bSuccess = file.FileOpen( szFileName );
	if ( pstrResult )
		*pstrResult = file.m_strIOResult;
	if ( bSuccess )
	{
		if ( GKStringISEMPTY(file.m_strEncoding) && ! GKStringISEMPTY(strDoc) )
		{
			file.m_strEncoding = GetDeclaredEncoding( strDoc );
			if ( GKStringISEMPTY(file.m_strEncoding) )
				file.m_strEncoding = _U("UTF-8"); // to do: MDF_ANSIFILE
		}
		file.FileSpecifyEncoding( pstrEncoding );
		bSuccess = file.FileWriteText( strDoc );
		file.FileClose();
		if ( pstrResult )
			*pstrResult += file.m_strIOResult;
		if ( pnDocFlags )
			*pnDocFlags = file.m_nDocFlags;
	}
	return bSuccess;
}

GKbool CMarkup::FindElem( GKPCSTR szName )
{
	if ( m_nDocFlags & MDF_WRITEFILE )
		return FALSE;
	if ( m_pElemPosTree->GetSize() )
	{
		// Change current position only if found
		PathPos path( szName, FALSE );
		GKint32 iPos = x_FindElem( m_iPosParent, m_iPos, path );
		if ( iPos )
		{
			// Assign new position
			x_SetPos( ELEM(iPos).iElemParent, iPos, 0 );
			return TRUE;
		}
	}
	return FALSE;
}

GKbool CMarkup::FindChildElem( GKPCSTR szName )
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	// Shorthand: if no current main position, find first child under parent element
	if ( ! m_iPos )
		FindElem();
	// Change current child position only if found
	PathPos path( szName, FALSE );
	GKint32 iPosChild = x_FindElem( m_iPos, m_iPosChild, path );
	if ( iPosChild )
	{
		// Assign new position
		GKint32 iPos = ELEM(iPosChild).iElemParent;
		x_SetPos( ELEM(iPos).iElemParent, iPos, iPosChild );
		return TRUE;
	}
	return FALSE;
}

GKString CMarkup::EscapeText( GKPCSTR szText, GKint32 nFlags )
{
	// Convert text as seen outside XML document to XML friendly
	// replacing special characters with ampersand escape codes
	// E.g. convert "6>7" to "6&gt;7"
	//
	// &lt;   less than
	// &amp;  ampersand
	// &gt;   greater than
	//
	// and for attributes:
	//
	// &apos; apostrophe or single quote
	// &quot; double quote
	//
	static GKPCSTR apReplace[] = { NULL,_U("&lt;"),_U("&amp;"),_U("&gt;"),_U("&quot;"),_U("&apos;") };
	GKString strText;
	GKPCSTR pSource = szText;
	GKint32 nDestSize = GKSTRLEN(pSource);
	nDestSize += nDestSize / 10 + 7;
	MCD_BLDRESERVE(strText,nDestSize);
	GKuchar cSource = *pSource;
	GKint32 nFound;
	GKint32 nCharLen;
	while ( cSource )
	{
		MCD_BLDCHECK(strText,nDestSize,6);
		nFound = ((nFlags&MNF_ESCAPEQUOTES)?x_ISATTRIBSPECIAL(cSource):x_ISSPECIAL(cSource));
		if ( nFound )
		{
			GKbool bIgnoreAmpersand = FALSE;
			if ( (nFlags&MNF_WITHREFS) && cSource == '&' )
			{
				// Do not replace ampersand if it is start of any entity reference
				// &[#_:A-Za-zU][_:-.A-Za-z0-9U]*; where U is > 0x7f
				GKPCSTR pCheckEntity = pSource;
				++pCheckEntity;
				GKuchar c = *pCheckEntity;
				if ( x_ISSTARTENTREF(c) || ((unsigned GKint32)c)>0x7f )
				{
					while ( 1 )
					{
						pCheckEntity += MCD_CLEN( pCheckEntity );
						c = *pCheckEntity;
						if ( c == ';' )
						{
							GKint32 nEntityLen = (GKint32)(pCheckEntity - pSource) + 1;
							MCD_BLDAPPENDN(strText,pSource,nEntityLen);
							pSource = pCheckEntity;
							bIgnoreAmpersand = TRUE;
						}
						else if ( x_ISINENTREF(c) || ((unsigned GKint32)c)>0x7f )
							continue;
						break;
					}
				}
			}
			if ( ! bIgnoreAmpersand )
			{
				MCD_BLDAPPEND(strText,apReplace[nFound]);
			}
			++pSource; // ASCII, so 1 byte
		}
		else
		{
			nCharLen = MCD_CLEN( pSource );
			MCD_BLDAPPENDN(strText,pSource,nCharLen);
			pSource += nCharLen;
		}
		cSource = *pSource;
	}

	MCD_BLDRELEASE(strText);
	return strText;
}

// Predefined character entities
// By default UnescapeText will decode standard HTML entities as well as the 5 in XML
// To unescape only the 5 standard XML entities, use this short table instead:
// GKPCSTR PredefEntityTable[4] =
// { _U("20060lt"),_U("40034quot"),_U("30038amp"),_U("20062gt40039apos") };
//
// This is a precompiled ASCII hash table for speed and minimum memory requirement
// Each entry consists of a 1 digit code name length, 4 digit code point, and the code name
// Each table slot can have multiple entries, table size 130 was chosen for even distribution
//
GKPCSTR PredefEntityTable[130] =
{
	_U("60216oslash60217ugrave60248oslash60249ugrave"),
	_U("50937omega60221yacute58968lceil50969omega60253yacute"),
	_U("50916delta50206icirc50948delta50238icirc68472weierp"),_U("40185sup1"),
	_U("68970lfloor40178sup2"),
	_U("50922kappa60164curren50954kappa58212mdash40179sup3"),
	_U("59830diams58211ndash"),_U("68855otimes58969rceil"),
	_U("50338oelig50212ocirc50244ocirc50339oelig58482trade"),
	_U("50197aring50931sigma50229aring50963sigma"),
	_U("50180acute68971rfloor50732tilde"),_U("68249lsaquo"),
	_U("58734infin68201thinsp"),_U("50161iexcl"),
	_U("50920theta50219ucirc50952theta50251ucirc"),_U("58254oline"),
	_U("58260frasl68727lowast"),_U("59827clubs60191iquest68250rsaquo"),
	_U("58629crarr50181micro"),_U("58222bdquo"),_U(""),
	_U("58243prime60177plusmn58242prime"),_U("40914beta40946beta"),_U(""),
	_U(""),_U(""),_U("50171laquo50215times"),_U("40710circ"),
	_U("49001lang"),_U("58220ldquo40175macr"),
	_U("40182para50163pound48476real"),_U(""),_U("58713notin50187raquo"),
	_U("48773cong50223szlig50978upsih"),
	_U("58776asymp58801equiv49002rang58218sbquo"),
	_U("50222thorn48659darr48595darr40402fnof58221rdquo50254thorn"),
	_U("40162cent58722minus"),_U("58707exist40170ordf"),_U(""),
	_U("40921iota58709empty48660harr48596harr40953iota"),_U(""),
	_U("40196auml40228auml48226bull40167sect48838sube"),_U(""),
	_U("48656larr48592larr58853oplus"),_U("30176deg58216lsquo40186ordm"),
	_U("40203euml40039apos40235euml48712isin40160nbsp"),
	_U("40918zeta40950zeta"),_U("38743and48195emsp48719prod"),
	_U("30935chi38745cap30967chi48194ensp"),
	_U("40207iuml40239iuml48706part48869perp48658rarr48594rarr"),
	_U("38736ang48836nsub58217rsquo"),_U(""),
	_U("48901sdot48657uarr48593uarr"),_U("40169copy48364euro"),
	_U("30919eta30951eta"),_U("40214ouml40246ouml48839supe"),_U(""),
	_U(""),_U("30038amp30174reg"),_U("48733prop"),_U(""),
	_U("30208eth30934phi40220uuml30240eth30966phi40252uuml"),_U(""),_U(""),
	_U(""),_U("40376yuml40255yuml"),_U(""),_U("40034quot48204zwnj"),
	_U("38746cup68756there4"),_U("30929rho30961rho38764sim"),
	_U("30932tau38834sub30964tau"),_U("38747int38206lrm38207rlm"),
	_U("30936psi30968psi30165yen"),_U(""),_U("28805ge30168uml"),
	_U("30982piv"),_U(""),_U("30172not"),_U(""),_U("28804le"),
	_U("30173shy"),_U("39674loz28800ne38721sum"),_U(""),_U(""),
	_U("38835sup"),_U("28715ni"),_U(""),_U("20928pi20960pi38205zwj"),
	_U(""),_U("60923lambda20062gt60955lambda"),_U(""),_U(""),
	_U("60199ccedil60231ccedil"),_U(""),_U("20060lt"),
	_U("20926xi28744or20958xi"),_U("20924mu20956mu"),_U("20925nu20957nu"),
	_U("68225dagger68224dagger"),_U("80977thetasym"),_U(""),_U(""),
	_U(""),_U("78501alefsym"),_U(""),_U(""),_U(""),
	_U("60193aacute60195atilde60225aacute60227atilde"),_U(""),
	_U("70927omicron60247divide70959omicron"),_U("60192agrave60224agrave"),
	_U("60201eacute60233eacute60962sigmaf"),_U("70917epsilon70949epsilon"),
	_U(""),_U("60200egrave60232egrave"),_U("60205iacute60237iacute"),
	_U(""),_U(""),_U("60204igrave68230hellip60236igrave"),
	_U("60166brvbar"),
	_U("60209ntilde68704forall58711nabla60241ntilde69824spades"),
	_U("60211oacute60213otilde60189frac1260183middot60243oacute60245otilde"),
	_U(""),_U("50184cedil60188frac14"),
	_U("50198aelig50194acirc60210ograve50226acirc50230aelig60242ograve"),
	_U("50915gamma60190frac3450947gamma58465image58730radic"),
	_U("60352scaron60353scaron"),_U("60218uacute69829hearts60250uacute"),
	_U("50913alpha50202ecirc70933upsilon50945alpha50234ecirc70965upsilon"),
	_U("68240permil")
};

GKString CMarkup::UnescapeText( GKPCSTR szText, GKint32 nTextLength /*=-1*/, GKint32 nFlags /*=0*/ )
{
	// Convert XML friendly text to text as seen outside XML document
	// ampersand escape codes replaced with special characters e.g. convert "6&gt;7" to "6>7"
	// ampersand numeric codes replaced with character e.g. convert &#60; to <
	// Conveniently the result is always the same or shorter in byte length
	//
	GKString strText;
	GKPCSTR pSource = szText;
	if ( nTextLength == -1 )
		nTextLength = GKSTRLEN(szText);
	MCD_BLDRESERVE(strText,nTextLength);
	GKuchar szCodeName[10];
	GKbool bAlterWhitespace = (nFlags & (MDF_TRIMWHITESPACE|MDF_COLLAPSEWHITESPACE))?TRUE:FALSE;
	GKbool bCollapseWhitespace = (nFlags & MDF_COLLAPSEWHITESPACE)?TRUE:FALSE;
	GKint32 nCharWhitespace = -1; // start of string
	GKint32 nCharLen;
	GKint32 nChar = 0;
	while ( nChar < nTextLength )
	{
		if ( pSource[nChar] == '&' )
		{
			if ( bAlterWhitespace )
				nCharWhitespace = 0;

			// Get corresponding unicode code point
			GKint32 nUnicode = 0;

			// Look for terminating semi-colon within 9 ASCII characters
			GKint32 nCodeLen = 0;
			GKuchar cCodeChar = pSource[nChar+1];
			while ( nCodeLen < 9 && ((unsigned GKint32)cCodeChar) < 128 && cCodeChar != ';' )
			{
				if ( cCodeChar >= 'A' && cCodeChar <= 'Z') // upper case?
					cCodeChar += ('a' - 'A'); // make lower case
				szCodeName[nCodeLen] = cCodeChar;
				++nCodeLen;
				cCodeChar = pSource[nChar+1+nCodeLen];
			}
			if ( cCodeChar == ';' ) // found semi-colon?
			{
				// Decode szCodeName
				szCodeName[nCodeLen] = '\0';
				if ( *szCodeName == '#' ) // numeric character reference?
				{
					// Is it a hex number?
					GKint32 nBase = 10; // decimal
					GKint32 nNumberOffset = 1; // after #
					if ( szCodeName[1] == 'x' )
					{
						nNumberOffset = 2; // after #x
						nBase = 16; // hex
					}
					nUnicode = MCD_PSZTOL( &szCodeName[nNumberOffset], NULL, nBase );
				}
				else // does not start with #
				{
					// Look for matching code name in PredefEntityTable
					GKPCSTR pEntry = PredefEntityTable[x_Hash(szCodeName,sizeof(PredefEntityTable)/sizeof(GKPCSTR))];
					while ( *pEntry )
					{
						// e.g. entry: 40039apos means length 4, code point 0039, code name apos
						GKint32 nEntryLen = (*pEntry - '0');
						++pEntry;
						GKPCSTR pCodePoint = pEntry;
						pEntry += 4;
						if ( nEntryLen == nCodeLen && x_StrNCmp(szCodeName,pEntry,nEntryLen) == 0 )
						{
							// Convert digits to integer up to code name which always starts with alpha 
							nUnicode = MCD_PSZTOL( pCodePoint, NULL, 10 );
							break;
						}
						pEntry += nEntryLen;
					}
				}
			}

			// If a code point found, encode it into text
			if ( nUnicode )
			{
				GKuchar szChar[5];
				nCharLen = 0;
#if defined(MARKUP_WCHAR) // WCHAR
#if MARKUP_SIZEOFWCHAR == 4 // sizeof(wchar_t) == 4
				szChar[0] = (GKuchar)nUnicode;
				nCharLen = 1;
#else // sizeof(wchar_t) == 2
				EncodeCharUTF16( nUnicode, (unsigned short*)szChar, nCharLen );
#endif
#elif defined(MARKUP_MBCS) // MBCS/double byte
#if defined(MARKUP_WINCONV)
				GKint32 nUsedDefaultChar = 0;
				wchar_t wszUTF16[2];
				EncodeCharUTF16( nUnicode, (unsigned short*)wszUTF16, nCharLen );
				nCharLen = WideCharToMultiByte( CP_ACP, 0, wszUTF16, nCharLen, szChar, 5, NULL, &nUsedDefaultChar );
				if ( nUsedDefaultChar || nCharLen <= 0 )
					nUnicode = 0;
#else // not WINCONV
				wchar_t wcUnicode = (wchar_t)nUnicode;
				nCharLen = wctomb( szChar, wcUnicode );
				if ( nCharLen <= 0 )
					nUnicode = 0;
#endif // not WINCONV
#else // not WCHAR and not MBCS/double byte
				EncodeCharUTF8( nUnicode, szChar, nCharLen );
#endif // not WCHAR and not MBCS/double byte
				// Increment index past ampersand semi-colon
				if ( nUnicode ) // must check since MBCS case can clear it
				{
					MCD_BLDAPPENDN(strText,szChar,nCharLen);
					nChar += nCodeLen + 2;
				}
			}
			if ( ! nUnicode )
			{
				// If the code is not converted, leave it as is
				MCD_BLDAPPEND1(strText,'&');
				++nChar;
			}
		}
		else if ( bAlterWhitespace && x_ISWHITESPACE(pSource[nChar]) )
		{
			if ( nCharWhitespace == 0 && bCollapseWhitespace )
			{
				nCharWhitespace = MCD_BLDLEN(strText);
				MCD_BLDAPPEND1(strText,' ');
			}
			else if ( nCharWhitespace != -1 && ! bCollapseWhitespace )
			{
				if ( nCharWhitespace == 0 )
					nCharWhitespace = MCD_BLDLEN(strText);
				MCD_BLDAPPEND1(strText,pSource[nChar]);
			}
			++nChar;
		}
		else // not &
		{
			if ( bAlterWhitespace )
				nCharWhitespace = 0;
			nCharLen = MCD_CLEN(&pSource[nChar]);
			MCD_BLDAPPENDN(strText,&pSource[nChar],nCharLen);
			nChar += nCharLen;
		}
	}
	if ( bAlterWhitespace && nCharWhitespace > 0 )
	{
		MCD_BLDTRUNC(strText,nCharWhitespace);
	}
	MCD_BLDRELEASE(strText);
	return strText;
}

GKbool CMarkup::DetectUTF8( const char* pText, GKint32 nTextLen, GKint32* pnNonASCII/*=NULL*/, GKbool* bErrorAtEnd/*=NULL*/ )
{
	// return TRUE if ASCII or all non-ASCII byte sequences are valid UTF-8 pattern:
	// ASCII   0xxxxxxx
	// 2-byte  110xxxxx 10xxxxxx
	// 3-byte  1110xxxx 10xxxxxx 10xxxxxx
	// 4-byte  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	// *pnNonASCII is set (if pnNonASCII is not NULL) to the number of non-ASCII UTF-8 sequences
	// or if an invalid UTF-8 sequence is found, to 1 + the valid non-ASCII sequences up to the invalid sequence
	// *bErrorAtEnd is set (if bErrorAtEnd is not NULL) to TRUE if the UTF-8 was cut off at the end in mid valid sequence
	GKint32 nUChar;
	if ( pnNonASCII )
		*pnNonASCII = 0;
	const char* pTextEnd = pText + nTextLen;
	while ( *pText && pText != pTextEnd )
	{
		if ( (unsigned char)(*pText) & 0x80 )
		{
			if ( pnNonASCII )
				++(*pnNonASCII);
			nUChar = DecodeCharUTF8( pText, pTextEnd );
			if ( nUChar == -1 )
			{
				if ( bErrorAtEnd )
					*bErrorAtEnd = (pTextEnd == pText)? TRUE:FALSE;
				return FALSE;
			}
		}
		else
			++pText;
	}
	if ( bErrorAtEnd )
		*bErrorAtEnd = FALSE;
	return TRUE;
}

GKint32 CMarkup::DecodeCharUTF8( const char*& pszUTF8, const char* pszUTF8End/*=NULL*/ )
{
	// Return Unicode code point and increment pszUTF8 past 1-4 bytes
	// pszUTF8End can be NULL if pszUTF8 is null terminated
	GKint32 nUChar = (unsigned char)*pszUTF8;
	++pszUTF8;
	if ( nUChar & 0x80 )
	{
		GKint32 nExtraChars;
		if ( ! (nUChar & 0x20) )
		{
			nExtraChars = 1;
			nUChar &= 0x1f;
		}
		else if ( ! (nUChar & 0x10) )
		{
			nExtraChars = 2;
			nUChar &= 0x0f;
		}
		else if ( ! (nUChar & 0x08) )
		{
			nExtraChars = 3;
			nUChar &= 0x07;
		}
		else
			return -1;
		while ( nExtraChars-- )
		{
			if ( pszUTF8 == pszUTF8End || ! (*pszUTF8 & 0x80) )
				return -1;
			nUChar = nUChar<<6;
			nUChar |= *pszUTF8 & 0x3f;
			++pszUTF8;
		}
	}
	return nUChar;
}

void CMarkup::EncodeCharUTF16( GKint32 nUChar, unsigned short* pwszUTF16, GKint32& nUTF16Len )
{
	// Write UTF-16 sequence to pwszUTF16 for Unicode code point nUChar and update nUTF16Len
	// Be sure pwszUTF16 has room for up to 2 wide chars
	if ( nUChar & ~0xffff )
	{
		if ( pwszUTF16 )
		{
			// Surrogate pair
			nUChar -= 0x10000;
			pwszUTF16[nUTF16Len++] = (unsigned short)(((nUChar>>10) & 0x3ff) | 0xd800); // W1
			pwszUTF16[nUTF16Len++] = (unsigned short)((nUChar & 0x3ff) | 0xdc00); // W2
		}
		else
			nUTF16Len += 2;
	}
	else
	{
		if ( pwszUTF16 )
			pwszUTF16[nUTF16Len++] = (unsigned short)nUChar;
		else
			++nUTF16Len;
	}
}

GKint32 CMarkup::DecodeCharUTF16( const unsigned short*& pwszUTF16, const unsigned short* pszUTF16End/*=NULL*/ )
{
	// Return Unicode code point and increment pwszUTF16 past 1 or 2 (if surrogrates) UTF-16 code points
	// pszUTF16End can be NULL if pszUTF16 is zero terminated
	GKint32 nUChar = *pwszUTF16;
	++pwszUTF16;
	if ( (nUChar & ~0x000007ff) == 0xd800 ) // W1
	{
		if ( pwszUTF16 == pszUTF16End || ! (*pwszUTF16) ) // W2
			return -1; // incorrect UTF-16
		nUChar = (((nUChar & 0x3ff) << 10) | (*pwszUTF16 & 0x3ff)) + 0x10000;
		++pwszUTF16;
	}
	return nUChar;
}

void CMarkup::EncodeCharUTF8( GKint32 nUChar, char* pszUTF8, GKint32& nUTF8Len )
{
	// Write UTF-8 sequence to pszUTF8 for Unicode code point nUChar and update nUTF8Len
	// Be sure pszUTF8 has room for up to 4 bytes
	if ( ! (nUChar & ~0x0000007f) ) // < 0x80
	{
		if ( pszUTF8 )
			pszUTF8[nUTF8Len++] = (char)nUChar;
		else
			++nUTF8Len;
	}
	else if ( ! (nUChar & ~0x000007ff) ) // < 0x800
	{
		if ( pszUTF8 )
		{
			pszUTF8[nUTF8Len++] = (char)(((nUChar&0x7c0)>>6)|0xc0);
			pszUTF8[nUTF8Len++] = (char)((nUChar&0x3f)|0x80);
		}
		else
			nUTF8Len += 2;
	}
	else if ( ! (nUChar & ~0x0000ffff) ) // < 0x10000
	{
		if ( pszUTF8 )
		{
			pszUTF8[nUTF8Len++] = (char)(((nUChar&0xf000)>>12)|0xe0);
			pszUTF8[nUTF8Len++] = (char)(((nUChar&0xfc0)>>6)|0x80);
			pszUTF8[nUTF8Len++] = (char)((nUChar&0x3f)|0x80);
		}
		else
			nUTF8Len += 3;
	}
	else // < 0x110000
	{
		if ( pszUTF8 )
		{
			pszUTF8[nUTF8Len++] = (char)(((nUChar&0x1c0000)>>18)|0xf0);
			pszUTF8[nUTF8Len++] = (char)(((nUChar&0x3f000)>>12)|0x80);
			pszUTF8[nUTF8Len++] = (char)(((nUChar&0xfc0)>>6)|0x80);
			pszUTF8[nUTF8Len++] = (char)((nUChar&0x3f)|0x80);
		}
		else
			nUTF8Len += 4;
	}
}

GKint32 CMarkup::UTF16To8( char* pszUTF8, const unsigned short* pwszUTF16, GKint32 nUTF8Count )
{
	// Supports the same arguments as wcstombs
	// the pwszUTF16 source must be a NULL-terminated UTF-16 string
	// if pszUTF8 is NULL, the number of bytes required is returned and nUTF8Count is ignored
	// otherwise pszUTF8 is filled with the result string and NULL-terminated if nUTF8Count allows
	// nUTF8Count is the byte size of pszUTF8 and must be large enough for the NULL if NULL desired
	// and the number of bytes (excluding NULL) is returned
	//
	GKint32 nUChar, nUTF8Len = 0;
	while ( *pwszUTF16 )
	{
		// Decode UTF-16
		nUChar = DecodeCharUTF16( pwszUTF16, NULL );
		if ( nUChar == -1 )
			nUChar = '?';

		// Encode UTF-8
		if ( pszUTF8 && nUTF8Len + 4 > nUTF8Count )
		{
			GKint32 nUTF8LenSoFar = nUTF8Len;
			EncodeCharUTF8( nUChar, NULL, nUTF8Len );
			if ( nUTF8Len > nUTF8Count )
				return nUTF8LenSoFar;
			nUTF8Len = nUTF8LenSoFar;
		}
		EncodeCharUTF8( nUChar, pszUTF8, nUTF8Len );
	}
	if ( pszUTF8 && nUTF8Len < nUTF8Count )
		pszUTF8[nUTF8Len] = 0;
	return nUTF8Len;
}

GKint32 CMarkup::UTF8To16( unsigned short* pwszUTF16, const char* pszUTF8, GKint32 nUTF8Count )
{
	// Supports the same arguments as mbstowcs
	// the pszUTF8 source must be a UTF-8 string which will be processed up to NULL-terminator or nUTF8Count
	// if pwszUTF16 is NULL, the number of UTF-16 chars required is returned
	// nUTF8Count is maximum UTF-8 bytes to convert and should include NULL if NULL desired in result
	// if pwszUTF16 is not NULL it is filled with the result string and it must be large enough
	// result will be NULL-terminated if NULL encountered in pszUTF8 before nUTF8Count
	// and the number of UTF-8 bytes converted is returned
	//
	const char* pszPosUTF8 = pszUTF8;
	const char* pszUTF8End = pszUTF8 + nUTF8Count;
	GKint32 nUChar, nUTF8Len = 0, nUTF16Len = 0;
	while ( pszPosUTF8 != pszUTF8End )
	{
		nUChar = DecodeCharUTF8( pszPosUTF8, pszUTF8End );
		if ( ! nUChar )
		{
			if ( pwszUTF16 )
				pwszUTF16[nUTF16Len] = 0;
			break;
		}
		else if ( nUChar == -1 )
			nUChar = '?';

		// Encode UTF-16
		EncodeCharUTF16( nUChar, pwszUTF16, nUTF16Len );
	}
	nUTF8Len = (GKint32)(pszPosUTF8 - pszUTF8);
	if ( ! pwszUTF16 )
		return nUTF16Len;
	return nUTF8Len;
}

#if ! defined(MARKUP_WCHAR) // not WCHAR
GKString CMarkup::UTF8ToA( GKPCSTR pszUTF8, GKint32* pnFailed/*=NULL*/ )
{
	// Converts from UTF-8 to locale ANSI charset
	GKString strANSI;
	GKint32 nMBLen = (GKint32)GKSTRLEN( pszUTF8 );
	if ( pnFailed )
		*pnFailed = 0;
	if ( nMBLen )
	{
		TextEncoding textencoding( _U("UTF-8"), (const void*)pszUTF8, nMBLen );
		textencoding.m_nToCount = nMBLen;
		GKuchar* pANSIBuffer = MCD_GETBUFFER(strANSI,textencoding.m_nToCount);
		nMBLen = textencoding.PerformConversion( (void*)pANSIBuffer );
		MCD_RELEASEBUFFER(strANSI,pANSIBuffer,nMBLen);
		if ( pnFailed )
			*pnFailed = textencoding.m_nFailedChars;
	}
	return strANSI;
}

GKString CMarkup::AToUTF8( GKPCSTR pszANSI )
{
	// Converts locale ANSI charset to UTF-8
	GKString strUTF8;
	GKint32 nMBLen = (GKint32)GKSTRLEN( pszANSI );
	if ( nMBLen )
	{
		TextEncoding textencoding( _U(""), (const void*)pszANSI, nMBLen );
		textencoding.m_nToCount = nMBLen * 4;
		GKuchar* pUTF8Buffer = MCD_GETBUFFER(strUTF8,textencoding.m_nToCount);
		nMBLen = textencoding.PerformConversion( (void*)pUTF8Buffer, _U("UTF-8") );
		MCD_RELEASEBUFFER(strUTF8,pUTF8Buffer,nMBLen);
	}
	return strUTF8;
}
#endif // not WCHAR

GKString CMarkup::GetDeclaredEncoding( GKPCSTR szDoc )
{
	// Extract encoding attribute from XML Declaration, or HTML meta charset
	GKString strEncoding;
	TokenPos token( szDoc, MDF_IGNORECASE );
	NodePos node;
	GKbool bHtml = FALSE;
	GKint32 nTypeFound = 0;
	while ( nTypeFound >= 0 )
	{
		nTypeFound = token.ParseNode( node );
		GKint32 nNext = token.m_nNext;
		if ( nTypeFound == MNT_PROCESSING_INSTRUCTION && node.nStart == 0 )
		{
			token.m_nNext = node.nStart + 2; // after <?
			if ( token.FindName() && token.Match(_U("xml")) )
			{
				// e.g. <?xml version="1.0" encoding="UTF-8"?>
				if ( token.FindAttrib(_U("encoding")) )
					strEncoding = token.GetTokenText();
				break;
			}
		}
		else if ( nTypeFound == 0 ) // end tag
		{
			// Check for end of HTML head
			token.m_nNext = node.nStart + 2; // after </
			if ( token.FindName() && token.Match(_U("head")) )
				break;
		}
		else if ( nTypeFound == MNT_ELEMENT )
		{
			token.m_nNext = node.nStart + 1; // after <
			token.FindName();
			if ( ! bHtml )
			{
				if ( ! token.Match(_U("html")) )
					break;
				bHtml = TRUE;
			}
			else if ( token.Match(_U("meta")) )
			{
				// e.g. <META http-equiv=Content-Type content="text/html; charset=UTF-8">
				GKint32 nAttribOffset = node.nStart + 1;
				token.m_nNext = nAttribOffset;
				if ( token.FindAttrib(_U("http-equiv")) && token.Match(_U("Content-Type")) )
				{
					token.m_nNext = nAttribOffset;
					if ( token.FindAttrib(_U("content")) )
					{
						GKint32 nContentEndOffset = token.m_nNext;
						token.m_nNext = token.m_nL;
						while ( token.m_nNext < nContentEndOffset && token.FindName() )
						{
							if ( token.Match(_U("charset")) && token.FindName() && token.Match(_U("=")) )
							{
								token.FindName();
								strEncoding = token.GetTokenText();
								break;
							}
						}
					}
					break;
				}
			}
		}
		token.m_nNext = nNext;
	}
	return strEncoding;
}

GKint32 CMarkup::GetEncodingCodePage( GKPCSTR pszEncoding )
{
	return x_GetEncodingCodePage( pszEncoding );
}

GKint32 CMarkup::FindNode( GKint32 nType )
{
	// Change current node position only if a node is found
	// If nType is 0 find any node, otherwise find node of type nType
	// Return type of node or 0 if not found

	// Determine where in document to start scanning for node
	GKint32 nNodeOffset = m_nNodeOffset;
	if ( m_nNodeType > MNT_ELEMENT )
	{
		// By-pass current node
		nNodeOffset += m_nNodeLength;
	}
	else // element or no current main position
	{
		// Set position to begin looking for node
		if ( m_iPos )
		{
			// After element
			nNodeOffset = ELEM(m_iPos).StartAfter();
		}
		else if ( m_iPosParent )
		{
			// Immediately after start tag of parent
			if ( ELEM(m_iPosParent).IsEmptyElement() )
				return 0;
			else
				nNodeOffset = ELEM(m_iPosParent).StartContent();
		}
	}

	// Get nodes until we find what we're looking for
	GKint32 nTypeFound = 0;
	GKint32 iPosNew = m_iPos;
	TokenPos token( m_strDoc, m_nDocFlags );
	NodePos node;
	token.m_nNext = nNodeOffset;
	do
	{
		nNodeOffset = token.m_nNext;
		nTypeFound = token.ParseNode( node );
		if ( nTypeFound == 0 )
		{
			// Check if we have reached the end of the parent element
			if ( m_iPosParent && nNodeOffset == ELEM(m_iPosParent).StartContent()
					+ ELEM(m_iPosParent).ContentLen() )
				return 0;
			nTypeFound = MNT_LONE_END_TAG; // otherwise it is a lone end tag
		}
		else if ( nTypeFound < 0 )
		{
			if ( nTypeFound == -2 ) // end of document
				return 0;
			// -1 is node error
			nTypeFound = MNT_NODE_ERROR;
		}
		else if ( nTypeFound == MNT_ELEMENT )
		{
			if ( iPosNew )
				iPosNew = ELEM(iPosNew).iElemNext;
			else
				iPosNew = ELEM(m_iPosParent).iElemChild;
			if ( ! iPosNew )
				return 0;
			if ( ! nType || (nType & nTypeFound) )
			{
				// Found element node, move position to this element
				x_SetPos( m_iPosParent, iPosNew, 0 );
				return m_nNodeType;
			}
			token.m_nNext = ELEM(iPosNew).StartAfter();
		}
	}
	while ( nType && ! (nType & nTypeFound) );

	m_iPos = iPosNew;
	m_iPosChild = 0;
	m_nNodeOffset = node.nStart;
	m_nNodeLength = node.nLength;
	m_nNodeType = nTypeFound;
	MARKUP_SETDEBUGSTATE;
	return m_nNodeType;
}

GKbool CMarkup::RemoveNode()
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	if ( m_iPos || m_nNodeLength )
	{
		x_RemoveNode( m_iPosParent, m_iPos, m_nNodeType, m_nNodeOffset, m_nNodeLength );
		m_iPosChild = 0;
		MARKUP_SETDEBUGSTATE;
		return TRUE;
	}
	return FALSE;
}

GKString CMarkup::GetTagName() const
{
	// Return the tag name at the current main position
	GKString strTagName;

	// This method is primarily for elements, however
	// it does return something for certain other nodes
	if ( m_nNodeLength )
	{
		switch ( m_nNodeType )
		{
		case MNT_PROCESSING_INSTRUCTION:
		case MNT_LONE_END_TAG:
			{
				// <?target or </tagname
				TokenPos token( m_strDoc, m_nDocFlags );
				token.m_nNext = m_nNodeOffset + 2;
				if ( token.FindName() )
					strTagName = token.GetTokenText();
			}
			break;
		case MNT_COMMENT:
			strTagName = _U("#comment");
			break;
		case MNT_CDATA_SECTION:
			strTagName = _U("#cdata-section");
			break;
		case MNT_DOCUMENT_TYPE:
			{
				// <!DOCTYPE name
				TokenPos token( m_strDoc, m_nDocFlags );
				token.m_nNext = m_nNodeOffset + 2;
				if ( token.FindName() && token.FindName() )
					strTagName = token.GetTokenText();
			}
			break;
		case MNT_TEXT:
		case MNT_WHITESPACE:
			strTagName = _U("#text");
			break;
		}
		return strTagName;
	}

	if ( m_iPos )
		strTagName = x_GetTagName( m_iPos );
	return strTagName;
}

GKbool CMarkup::IntoElem()
{
	// Make current element the parent
	if ( m_iPos && m_nNodeType == MNT_ELEMENT )
	{
		x_SetPos( m_iPos, m_iPosChild, 0 );
		return TRUE;
	}
	return FALSE;
}

GKbool CMarkup::OutOfElem()
{
	// Go to parent element
	if ( m_iPosParent )
	{
		x_SetPos( ELEM(m_iPosParent).iElemParent, m_iPosParent, m_iPos );
		return TRUE;
	}
	return FALSE;
}

GKbool CMarkup::GetNthAttrib( GKint32 n, GKString& strAttrib, GKString& strValue ) const
{
	// Return nth attribute name and value from main position
	TokenPos token( m_strDoc, m_nDocFlags );
	if ( m_iPos && m_nNodeType == MNT_ELEMENT )
		token.m_nNext = ELEM(m_iPos).nStart + 1;
	else if ( m_nNodeLength && m_nNodeType == MNT_PROCESSING_INSTRUCTION )
		token.m_nNext = m_nNodeOffset + 2;
	else
		return FALSE;
	if ( token.FindAttrib(NULL,n,&strAttrib) )
	{
		strValue = UnescapeText( token.GetTokenPtr(), token.Length(), m_nDocFlags );
		return TRUE;
	}
	return FALSE;
}

GKString CMarkup::GetAttribName( GKint32 n ) const
{
	// Return nth attribute name of main position
	TokenPos token( m_strDoc, m_nDocFlags );
	if ( m_iPos && m_nNodeType == MNT_ELEMENT )
		token.m_nNext = ELEM(m_iPos).nStart + 1;
	else if ( m_nNodeLength && m_nNodeType == MNT_PROCESSING_INSTRUCTION )
		token.m_nNext = m_nNodeOffset + 2;
	else
		return _U("");
	if ( token.FindAttrib(NULL,n) )
		return token.GetTokenText();
	return _U("");
}

GKbool CMarkup::SavePos( GKPCSTR szPosName /*=""*/, GKint32 nMap /*=0*/ )
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	// Save current element position in saved position map
	if ( szPosName )
	{
		SavedPosMap* pMap;
		m_pSavedPosMaps->GetMap( pMap, nMap );
		SavedPos savedpos;
		if ( szPosName )
			savedpos.strName = szPosName;
		if ( m_iPosChild )
		{
			savedpos.iPos = m_iPosChild;
			savedpos.nSavedPosFlags |= SavedPos::SPM_CHILD;
		}
		else if ( m_iPos )
		{
			savedpos.iPos = m_iPos;
			savedpos.nSavedPosFlags |= SavedPos::SPM_MAIN;
		}
		else
		{
			savedpos.iPos = m_iPosParent;
		}
		savedpos.nSavedPosFlags |= SavedPos::SPM_USED;

		GKint32 nSlot = x_Hash( szPosName, pMap->nMapSize);
		SavedPos* pSavedPos = pMap->pTable[nSlot];
		GKint32 nOffset = 0;
		if ( ! pSavedPos )
		{
			pSavedPos = new SavedPos[2];
			pSavedPos[1].nSavedPosFlags = SavedPos::SPM_LAST;
			pMap->pTable[nSlot] = pSavedPos;
		}
		else
		{
			while ( pSavedPos[nOffset].nSavedPosFlags & SavedPos::SPM_USED )
			{
				if ( pSavedPos[nOffset].strName == (GKPCSTR)szPosName )
					break;
				if ( pSavedPos[nOffset].nSavedPosFlags & SavedPos::SPM_LAST )
				{
					GKint32 nNewSize = (nOffset + 6) * 2;
					SavedPos* pNewSavedPos = new SavedPos[nNewSize];
					for ( GKint32 nCopy=0; nCopy<=nOffset; ++nCopy )
						pNewSavedPos[nCopy] = pSavedPos[nCopy];
					pNewSavedPos[nOffset].nSavedPosFlags ^= SavedPos::SPM_LAST;
					pNewSavedPos[nNewSize-1].nSavedPosFlags = SavedPos::SPM_LAST;
					delete [] pSavedPos;
					pSavedPos = pNewSavedPos;
					pMap->pTable[nSlot] = pSavedPos;
					++nOffset;
					break;
				}
				++nOffset;
			}
		}
		if ( pSavedPos[nOffset].nSavedPosFlags & SavedPos::SPM_LAST )
			savedpos.nSavedPosFlags |= SavedPos::SPM_LAST;
		pSavedPos[nOffset] = savedpos;

		/*
		// To review hash table balance, uncomment and watch strBalance
		GKString strBalance, strSlot;
		for ( nSlot=0; nSlot < pMap->nMapSize; ++nSlot )
		{
			pSavedPos = pMap->pTable[nSlot];
			GKint32 nCount = 0;
			while ( pSavedPos && pSavedPos->nSavedPosFlags & SavedPos::SPM_USED )
			{
				++nCount;
				if ( pSavedPos->nSavedPosFlags & SavedPos::SPM_LAST )
					break;
				++pSavedPos;
			}
			strSlot.Format( _U("%d "), nCount );
			strBalance += strSlot;
		}
		*/
		return TRUE;
	}
	return FALSE;
}

GKbool CMarkup::RestorePos( GKPCSTR szPosName /*=""*/, GKint32 nMap /*=0*/ )
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	// Restore element position if found in saved position map
	if ( szPosName )
	{
		SavedPosMap* pMap;
		m_pSavedPosMaps->GetMap( pMap, nMap );
		GKint32 nSlot = x_Hash( szPosName, pMap->nMapSize );
		SavedPos* pSavedPos = pMap->pTable[nSlot];
		if ( pSavedPos )
		{
			GKint32 nOffset = 0;
			while ( pSavedPos[nOffset].nSavedPosFlags & SavedPos::SPM_USED )
			{
				if ( pSavedPos[nOffset].strName == (GKPCSTR)szPosName )
				{
					GKint32 i = pSavedPos[nOffset].iPos;
					if ( pSavedPos[nOffset].nSavedPosFlags & SavedPos::SPM_CHILD )
						x_SetPos( ELEM(ELEM(i).iElemParent).iElemParent, ELEM(i).iElemParent, i );
					else if ( pSavedPos[nOffset].nSavedPosFlags & SavedPos::SPM_MAIN )
						x_SetPos( ELEM(i).iElemParent, i, 0 );
					else
						x_SetPos( i, 0, 0 );
					return TRUE;
				}
				if ( pSavedPos[nOffset].nSavedPosFlags & SavedPos::SPM_LAST )
					break;
				++nOffset;
			}
		}
	}
	return FALSE;
}

GKbool CMarkup::SetMapSize( GKint32 nSize, GKint32 nMap /*=0*/ )
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	// Set saved position map hash table size before using it
	// Returns FALSE if map already exists
	// Some prime numbers: 53, 101, 211, 503, 1009, 2003, 10007, 20011, 50021, 100003, 200003, 500009
	SavedPosMap* pNewMap;
	return m_pSavedPosMaps->GetMap( pNewMap, nMap, nSize );
}

GKbool CMarkup::RemoveElem()
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	// Remove current main position element
	if ( m_iPos && m_nNodeType == MNT_ELEMENT )
	{
		GKint32 iPos = x_RemoveElem( m_iPos );
		x_SetPos( m_iPosParent, iPos, 0 );
		return TRUE;
	}
	return FALSE;
}

GKbool CMarkup::RemoveChildElem()
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	// Remove current child position element
	if ( m_iPosChild )
	{
		GKint32 iPosChild = x_RemoveElem( m_iPosChild );
		x_SetPos( m_iPosParent, m_iPos, iPosChild );
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CMarkup private methods
//
void CMarkup::x_InitMarkup()
{
	// Only called from CMarkup constructors
	m_pFilePos = NULL;
	m_pSavedPosMaps = new SavedPosMapArray;
	m_pElemPosTree = new ElemPosTree;

	// To always ignore case, define MARKUP_IGNORECASE
#if defined(MARKUP_IGNORECASE) // ignore case
	m_nDocFlags = MDF_IGNORECASE;
#else // not ignore case
	m_nDocFlags = 0;
#endif // not ignore case
}

GKint32 CMarkup::x_GetParent( GKint32 i )
{
	return ELEM(i).iElemParent;
}

void CMarkup::x_SetPos( GKint32 iPosParent, GKint32 iPos, GKint32 iPosChild )
{
	m_iPosParent = iPosParent;
	m_iPos = iPos;
	m_iPosChild = iPosChild;
	m_nNodeOffset = 0;
	m_nNodeLength = 0;
	m_nNodeType = iPos?MNT_ELEMENT:0;
	MARKUP_SETDEBUGSTATE;
}

#if defined(_DEBUG) // DEBUG 
void CMarkup::x_SetDebugState()
{
	// Set m_pDebugCur and m_pDebugPos to point into document
	GKPCSTR pD = MCD_2PCSZ(m_strDoc);

	// Node (non-element) position is determined differently in file mode
	if ( m_nNodeLength || (m_nNodeOffset && !m_pFilePos)
			|| (m_pFilePos && (!m_iPos) && (!m_iPosParent) && ! m_pFilePos->FileAtTop()) )
	{
		if ( ! m_nNodeLength )
			m_pDebugCur = _U("main position offset"); // file mode only
		else
			m_pDebugCur = _U("main position node");
		m_pDebugPos = &pD[m_nNodeOffset];
	}
	else
	{
		if ( m_iPosChild )
		{
			m_pDebugCur = _U("child position element");
			m_pDebugPos = &pD[ELEM(m_iPosChild).nStart];
		}
		else if ( m_iPos )
		{
			m_pDebugCur = _U("main position element");
			m_pDebugPos = &pD[ELEM(m_iPos).nStart];
		}
		else if ( m_iPosParent )
		{
			m_pDebugCur = _U("parent position element");
			m_pDebugPos = &pD[ELEM(m_iPosParent).nStart];
		}
		else
		{
			m_pDebugCur = _U("top of document");
			m_pDebugPos = pD;
		}
	}
}
#endif // DEBUG

GKint32 CMarkup::x_GetFreePos()
{
	if ( m_iPosFree == m_pElemPosTree->GetSize() )
		x_AllocElemPos();
	return m_iPosFree++;
}

GKbool CMarkup::x_AllocElemPos( GKint32 nNewSize /*=0*/ )
{
	// Resize m_aPos when the document is created or the array is filled
	if ( ! nNewSize )
		nNewSize = m_iPosFree + (m_iPosFree>>1); // Grow By: multiply size by 1.5
	if ( m_pElemPosTree->GetSize() < nNewSize )
		m_pElemPosTree->GrowElemPosTree( nNewSize );
	return TRUE;
}

GKbool CMarkup::x_ParseDoc()
{
	// Reset indexes
	ResetPos();
	m_pSavedPosMaps->ReleaseMaps();

	// Starting size of position array: 1 element per 64 bytes of document
	// Tight fit when parsing small doc, only 0 to 2 reallocs when parsing large doc
	// Start at 8 when creating new document
	GKint32 nDocLen = m_strDoc.GetLength();
	m_iPosFree = 1;
	x_AllocElemPos( nDocLen / 64 + 8 );
	m_iPosDeleted = 0;

	// Parse document
	ELEM(0).ClearVirtualParent();
	if ( nDocLen )
	{
		TokenPos token( m_strDoc, m_nDocFlags );
		GKint32 iPos = x_ParseElem( 0, token );
		ELEM(0).nLength = nDocLen;
		if ( iPos > 0 )
		{
			ELEM(0).iElemChild = iPos;
			if ( ELEM(iPos).iElemNext )
				x_AddResult( m_strResult, _U("root_has_sibling") );
		}
		else
			x_AddResult( m_strResult, _U("no_root_element") );
	}

	ResetPos();
	return IsWellFormed();
}

GKint32 CMarkup::x_ParseElem( GKint32 iPosParent, TokenPos& token )
{
	// This is either called by x_ParseDoc or x_AddSubDoc or x_SetElemContent
	// Returns index of the first element encountered or zero if no elements
	//
	GKint32 iPosRoot = 0;
	GKint32 iPos = iPosParent;
	GKint32 iVirtualParent = iPosParent;
	GKint32 nRootDepth = ELEM(iPos).Level();
	GKint32 nMatchLevel;
	GKint32 iPosMatch;
	GKint32 iTag;
	GKint32 nTypeFound;
	GKint32 iPosFirst;
	GKint32 iPosLast;
	ElemPos* pElem;
	ElemPos* pElemParent;
	ElemPos* pElemChild;

	// Loop through the nodes of the document
	ElemStack elemstack;
	NodePos node;
	token.m_nNext = 0;
	while ( 1 )
	{
		nTypeFound = token.ParseNode( node );
		nMatchLevel = 0;
		if ( nTypeFound == MNT_ELEMENT ) // start tag
		{
			iPos = x_GetFreePos();
			if ( ! iPosRoot )
				iPosRoot = iPos;
			pElem = &ELEM(iPos);
			pElem->iElemParent = iPosParent;
			pElem->iElemNext = 0;
			pElemParent = &ELEM(iPosParent);
			if ( pElemParent->iElemChild )
			{
				iPosFirst = pElemParent->iElemChild;
				pElemChild = &ELEM(iPosFirst);
				iPosLast = pElemChild->iElemPrev;
				ELEM(iPosLast).iElemNext = iPos;
				pElem->iElemPrev = iPosLast;
				pElemChild->iElemPrev = iPos;
				pElem->nFlags = 0;
			}
			else
			{
				pElemParent->iElemChild = iPos;
				pElem->iElemPrev = iPos;
				pElem->nFlags = MNF_FIRST;
			}
			pElem->SetLevel( nRootDepth + elemstack.iTop );
			pElem->iElemChild = 0;
			pElem->nStart = node.nStart;
			pElem->SetStartTagLen( node.nLength );
			if ( node.nNodeFlags & MNF_EMPTY )
			{
				iPos = iPosParent;
				pElem->SetEndTagLen( 0 );
				pElem->nLength = node.nLength;
			}
			else
			{
				iPosParent = iPos;
				elemstack.PushIntoLevel( token.GetTokenPtr(), token.Length() );
			}
		}
		else if ( nTypeFound == 0 ) // end tag
		{
			iPosMatch = iPos;
			iTag = elemstack.iTop;
			nMatchLevel = iTag;
			while ( nMatchLevel && ! token.Match(elemstack.GetRefTagPosAt(iTag--).strTagName) )
			{
				--nMatchLevel;
				iPosMatch = ELEM(iPosMatch).iElemParent;
			}
			if ( nMatchLevel == 0 )
			{
				// Not matched at all, it is a lone end tag, a non-element node
				ELEM(iVirtualParent).nFlags |= MNF_ILLFORMED;
				ELEM(iPos).nFlags |= MNF_ILLDATA;
				x_AddResult( m_strResult, _U("lone_end_tag"), token.GetTokenText(), 0, node.nStart );
			}
			else
			{
				pElem = &ELEM(iPosMatch);
				pElem->nLength = node.nStart - pElem->nStart + node.nLength;
				pElem->SetEndTagLen( node.nLength );
			}
		}
		else if ( nTypeFound == -1 )
		{
			ELEM(iVirtualParent).nFlags |= MNF_ILLFORMED;
			ELEM(iPos).nFlags |= MNF_ILLDATA;
			m_strResult += node.strMeta;
		}

		// Matched end tag, or end of document
		if ( nMatchLevel || nTypeFound == -2 )
		{
			if ( elemstack.iTop > nMatchLevel )
				ELEM(iVirtualParent).nFlags |= MNF_ILLFORMED;

			// Process any non-ended elements
			while ( elemstack.iTop > nMatchLevel )
			{
				// Element with no end tag
				pElem = &ELEM(iPos);
				GKint32 iPosChild = pElem->iElemChild;
				iPosParent = pElem->iElemParent;
				pElem->SetEndTagLen( 0 );
				pElem->nFlags |= MNF_NONENDED;
				pElem->iElemChild = 0;
				pElem->nLength = pElem->StartTagLen();
				if ( pElem->nFlags & MNF_ILLDATA )
				{
					pElem->nFlags ^= MNF_ILLDATA;
					ELEM(iPosParent).nFlags |= MNF_ILLDATA;
				}
				while ( iPosChild )
				{
					ELEM(iPosChild).iElemParent = iPosParent;
					ELEM(iPosChild).iElemPrev = iPos;
					ELEM(iPos).iElemNext = iPosChild;
					iPos = iPosChild;
					iPosChild = ELEM(iPosChild).iElemNext;
				}

				// If end tag did not match, top node is end tag that did not match pElem
				// if end of document, any nodes below top have no end tag
				// second offset represents location where end tag was expected but end of document or other end tag was found 
				// end tag that was found is token.GetTokenText() but not reported in error
				GKint32 nOffset2 = (nTypeFound==0)? token.m_nL-1: m_strDoc.GetLength();
				x_AddResult( m_strResult, _U("unended_start_tag"), elemstack.Current().strTagName, 0, pElem->nStart, nOffset2 );

				iPos = iPosParent;
				elemstack.PopOutOfLevel();
			}
			if ( nTypeFound == -2 )
				break;
			iPosParent = ELEM(iPos).iElemParent;
			iPos = iPosParent;
			elemstack.PopOutOfLevel();
		}
	}
	return iPosRoot;
}

GKint32 CMarkup::x_FindElem( GKint32 iPosParent, GKint32 iPos, PathPos& path ) const
{
	// If pPath is NULL or empty, go to next sibling element
	// Otherwise go to next sibling element with matching path
	//
	if ( ! path.ValidPath() )
		return 0;

	// Paths other than simple tag name are only supported in the developer version
	if ( path.IsAnywherePath() || path.IsAbsolutePath() )
		return 0;

	if ( iPos )
		iPos = ELEM(iPos).iElemNext;
	else
		iPos = ELEM(iPosParent).iElemChild;

	// Finished here if pPath not specified
	if ( ! path.IsPath() )
		return iPos;

	// Search
	TokenPos token( m_strDoc, m_nDocFlags );
	while ( iPos )
	{
		// Compare tag name
		token.m_nNext = ELEM(iPos).nStart + 1;
		token.FindName(); // Locate tag name
		if ( token.Match(path.GetPtr()) )
			return iPos;
		iPos = ELEM(iPos).iElemNext;
	}
	return 0;

}

GKString CMarkup::x_GetPath( GKint32 iPos ) const
{
	// In file mode, iPos is an index into m_pFilePos->m_elemstack or zero
	GKString strPath;
	while ( iPos )
	{
		GKString strTagName;
		GKint32 iPosParent;
		GKint32 nCount = 0;
		if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		{
			TagPos& tag = m_pFilePos->m_elemstack.GetRefTagPosAt(iPos);
			strTagName = tag.strTagName;
			nCount = tag.nCount;
			iPosParent = tag.iParent;
		}
		else
		{
			strTagName = x_GetTagName( iPos );
			PathPos path( MCD_2PCSZ(strTagName), FALSE );
			iPosParent = ELEM(iPos).iElemParent;
			GKint32 iPosSib = 0;
			while ( iPosSib != iPos )
			{
				path.RevertOffset();
				iPosSib = x_FindElem( iPosParent, iPosSib, path );
				++nCount;
			}
		}
		if ( nCount == 1 )
			strPath = _U("/") + strTagName + strPath;
		else
		{
			GKuchar szPred[25];
			MCD_SPRINTF( MCD_SSZ(szPred), _U("[%d]"), nCount );
			strPath = _U("/") + strTagName + szPred + strPath;
		}
		iPos = iPosParent;
	}
	return strPath;
}

GKString CMarkup::x_GetTagName( GKint32 iPos ) const
{
	// Return the tag name at specified element
	TokenPos token( m_strDoc, m_nDocFlags );
	token.m_nNext = ELEM(iPos).nStart + 1;
	if ( ! iPos || ! token.FindName() )
		return _U("");

	// Return substring of document
	return token.GetTokenText();
}

GKString CMarkup::x_GetAttrib( GKint32 iPos, GKPCSTR pAttrib ) const
{
	// Return the value of the attrib
	TokenPos token( m_strDoc, m_nDocFlags );
	if ( iPos && m_nNodeType == MNT_ELEMENT )
		token.m_nNext = ELEM(iPos).nStart + 1;
	else if ( iPos == m_iPos && m_nNodeLength && m_nNodeType == MNT_PROCESSING_INSTRUCTION )
		token.m_nNext = m_nNodeOffset + 2;
	else
		return _U("");

	if ( pAttrib && token.FindAttrib(pAttrib) )
		return UnescapeText( token.GetTokenPtr(), token.Length(), m_nDocFlags );
	return _U("");
}

GKbool CMarkup::x_SetAttrib( GKint32 iPos, GKPCSTR pAttrib, GKint32 nValue, GKint32 nFlags /*=0*/ )
{
	// Convert integer to string
	GKuchar szVal[25];
	MCD_SPRINTF( MCD_SSZ(szVal), _U("%d"), nValue );
	return x_SetAttrib( iPos, pAttrib, szVal, nFlags );
}

GKbool CMarkup::x_SetAttrib( GKint32 iPos, GKPCSTR pAttrib, GKPCSTR pValue, GKint32 nFlags /*=0*/ )
{
	if ( m_nDocFlags & MDF_READFILE )
		return FALSE;
	GKint32 nNodeStart = 0;
	if ( iPos && m_nNodeType == MNT_ELEMENT )
		nNodeStart = ELEM(iPos).nStart;
	else if ( iPos == m_iPos && m_nNodeLength && m_nNodeType == MNT_PROCESSING_INSTRUCTION )
		nNodeStart = m_nNodeOffset;
	else
		return FALSE;

	// Create insertion text depending on whether attribute already exists
	// Decision: for empty value leaving attrib="" instead of removing attrib
	TokenPos token( m_strDoc, m_nDocFlags );
	token.m_nNext = nNodeStart + ((m_nNodeType == MNT_ELEMENT)?1:2); 
	GKint32 nReplace = 0;
	GKint32 nInsertAt;
	GKString strEscapedValue = EscapeText( pValue, MNF_ESCAPEQUOTES|nFlags );
	GKint32 nEscapedValueLen = strEscapedValue .GetLength();
	GKString strInsert;
	if ( token.FindAttrib(pAttrib) )
	{
		// Replace value
		MCD_BLDRESERVE( strInsert, nEscapedValueLen + 2 );
		MCD_BLDAPPEND1( strInsert, x_ATTRIBQUOTE );
		MCD_BLDAPPENDN( strInsert, MCD_2PCSZ(strEscapedValue), nEscapedValueLen );
		MCD_BLDAPPEND1( strInsert, x_ATTRIBQUOTE );
		MCD_BLDRELEASE( strInsert );
		nInsertAt = token.m_nL - ((token.m_nTokenFlags&MNF_QUOTED)?1:0);
		nReplace = token.Length() + ((token.m_nTokenFlags&MNF_QUOTED)?2:0);
	}
	else
	{
		// Insert string name value pair
		GKint32 nAttribNameLen = GKSTRLEN( pAttrib );
		MCD_BLDRESERVE( strInsert, nAttribNameLen + nEscapedValueLen + 4 );
		MCD_BLDAPPEND1( strInsert, ' ' );
		MCD_BLDAPPENDN( strInsert, pAttrib, nAttribNameLen );
		MCD_BLDAPPEND1( strInsert, '=' );
		MCD_BLDAPPEND1( strInsert, x_ATTRIBQUOTE );
		MCD_BLDAPPENDN( strInsert, MCD_2PCSZ(strEscapedValue), nEscapedValueLen );
		MCD_BLDAPPEND1( strInsert, x_ATTRIBQUOTE );
		MCD_BLDRELEASE( strInsert );
		nInsertAt = token.m_nNext;
	}

	GKint32 nAdjust = strInsert.GetLength() - nReplace;
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		GKint32 nNewDocLength = m_strDoc.GetLength() + nAdjust;
		m_strResult.Empty();
		if ( nNodeStart && nNewDocLength > m_pFilePos->m_nBlockSizeBasis )
		{
			GKint32 nDocCapacity = GKStringCAPACITY(m_strDoc);
			if ( nNewDocLength > nDocCapacity )
			{
				m_pFilePos->FileFlush( *m_pFilePos->m_pstrBuffer, nNodeStart );
				m_strResult = m_pFilePos->m_strIOResult;
				nInsertAt -= nNodeStart;
				m_nNodeOffset = 0;
				if ( m_nNodeType == MNT_ELEMENT )
					ELEM(iPos).nStart = 0;
			}
		}
	}
	x_DocChange( nInsertAt, nReplace, strInsert );
	if ( m_nNodeType == MNT_PROCESSING_INSTRUCTION )
	{
		x_AdjustForNode( m_iPosParent, m_iPos, nAdjust );
		m_nNodeLength += nAdjust;
	}
	else
	{
		ELEM(iPos).AdjustStartTagLen( nAdjust );
		ELEM(iPos).nLength += nAdjust;
		x_Adjust( iPos, nAdjust );
	}
	MARKUP_SETDEBUGSTATE;
	return TRUE;
}


GKbool CMarkup::x_CreateNode( GKString& strNode, GKint32 nNodeType, GKPCSTR pText )
{
	// Set strNode based on nNodeType and szData
	// Return FALSE if szData would jeopardize well-formed document
	//
	switch ( nNodeType )
	{
	case MNT_PROCESSING_INSTRUCTION:
		strNode = _U("<?");
		strNode += pText;
		strNode += _U("?>");
		break;
	case MNT_COMMENT:
		strNode = _U("<!--");
		strNode += pText;
		strNode += _U("-->");
		break;
	case MNT_ELEMENT:
		strNode = _U("<");
		strNode += pText;
		strNode += _U("/>");
		break;
	case MNT_TEXT:
	case MNT_WHITESPACE:
		strNode = EscapeText( pText );
		break;
	case MNT_DOCUMENT_TYPE:
		strNode = pText;
		break;
	case MNT_LONE_END_TAG:
		strNode = _U("</");
		strNode += pText;
		strNode += _U(">");
		break;
	case MNT_CDATA_SECTION:
		if ( MCD_PSZSTR(pText,_U("]]>")) != NULL )
			return FALSE;
		strNode = _U("<![CDATA[");
		strNode += pText;
		strNode += _U("]]>");
		break;
	}
	return TRUE;
}

GKString CMarkup::x_EncodeCDATASection( GKPCSTR szData )
{
	// Split CDATA Sections if there are any end delimiters
	GKString strData = _U("<![CDATA[");
	GKPCSTR pszNextStart = szData;
	GKPCSTR pszEnd = MCD_PSZSTR( szData, _U("]]>") );
	while ( pszEnd )
	{
		strData += GKString( pszNextStart, (GKint32)(pszEnd - pszNextStart) );
		strData += _U("]]]]><![CDATA[>");
		pszNextStart = pszEnd + 3;
		pszEnd = MCD_PSZSTR( pszNextStart, _U("]]>") );
	}
	strData += pszNextStart;
	strData += _U("]]>");
	return strData;
}

GKbool CMarkup::x_SetData( GKint32 iPos, GKint32 nValue )
{
	// Convert integer to string
	GKuchar szVal[25];
	MCD_SPRINTF( MCD_SSZ(szVal), _U("%d"), nValue );
	return x_SetData( iPos, szVal, 0 );
}

GKbool CMarkup::x_SetData( GKint32 iPos, GKPCSTR szData, GKint32 nFlags )
{
	if ( m_nDocFlags & MDF_READFILE )
		return FALSE;
	GKString strInsert;
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		if ( ! iPos || m_nNodeType != 1 || ! ELEM(iPos).IsEmptyElement() )
			return FALSE; // only set data on current empty element (no other kinds of nodes)
	}
	if ( iPos == m_iPos && m_nNodeLength )
	{
		// Not an element
		if ( ! x_CreateNode(strInsert, m_nNodeType, szData) )
			return FALSE;
		x_DocChange( m_nNodeOffset, m_nNodeLength, strInsert );
		x_AdjustForNode( m_iPosParent, iPos, strInsert.GetLength() - m_nNodeLength );
		m_nNodeLength = strInsert.GetLength();
		MARKUP_SETDEBUGSTATE;
		return TRUE;
	}

	// Set data in iPos element
	if ( ! iPos || ELEM(iPos).iElemChild )
		return FALSE;

	// Build strInsert from szData based on nFlags
	if ( nFlags & MNF_WITHCDATA )
		strInsert = x_EncodeCDATASection( szData );
	else
		strInsert = EscapeText( szData, nFlags );

	// Insert
	NodePos node( MNF_WITHNOLINES|MNF_REPLACE );
	node.strMeta = strInsert;
	GKint32 iPosBefore = 0;
	GKint32 nReplace = x_InsertNew( iPos, iPosBefore, node );
	GKint32 nAdjust = node.strMeta.GetLength() - nReplace;
	x_Adjust( iPos, nAdjust );
	ELEM(iPos).nLength += nAdjust;
	if ( ELEM(iPos).nFlags & MNF_ILLDATA )
		ELEM(iPos).nFlags &= ~MNF_ILLDATA;
	MARKUP_SETDEBUGSTATE;
	return TRUE;
}

GKString CMarkup::x_GetData( GKint32 iPos )
{
	if ( iPos == m_iPos && m_nNodeLength )
	{
		if ( m_nNodeType == MNT_COMMENT )
			return GKStringMID( m_strDoc, m_nNodeOffset+4, m_nNodeLength-7 );
		else if ( m_nNodeType == MNT_PROCESSING_INSTRUCTION )
			return GKStringMID( m_strDoc, m_nNodeOffset+2, m_nNodeLength-4 );
		else if ( m_nNodeType == MNT_CDATA_SECTION )
			return GKStringMID( m_strDoc, m_nNodeOffset+9, m_nNodeLength-12 );
		else if ( m_nNodeType == MNT_TEXT )
			return UnescapeText( &(MCD_2PCSZ(m_strDoc))[m_nNodeOffset], m_nNodeLength, m_nDocFlags );
		else if ( m_nNodeType == MNT_LONE_END_TAG )
			return GKStringMID( m_strDoc, m_nNodeOffset+2, m_nNodeLength-3 );
		return GKStringMID( m_strDoc, m_nNodeOffset, m_nNodeLength );
	}

	// Return a string representing data between start and end tag
	// Return empty string if there are any children elements
	GKString strData;
	if ( iPos && ! ELEM(iPos).IsEmptyElement() )
	{
		ElemPos* pElem = &ELEM(iPos);
		GKint32 nStartContent = pElem->StartContent();
		if ( pElem->IsUnparsed() )
		{
			TokenPos token( m_strDoc, m_nDocFlags, m_pFilePos );
			token.m_nNext = nStartContent;
			NodePos node;
			m_pFilePos->m_nReadBufferStart = pElem->nStart;
			while ( 1 )
			{
				m_pFilePos->m_nReadBufferRemoved = 0; // will be non-zero after ParseNode if read buffer shifted
				token.ParseNode( node );
				if ( m_pFilePos->m_nReadBufferRemoved )
				{
					pElem->nStart = 0;
					MARKUP_SETDEBUGSTATE;
				}
				if ( node.nNodeType == MNT_TEXT )
					strData += UnescapeText( &token.m_pDocText[node.nStart], node.nLength, m_nDocFlags );
				else if ( node.nNodeType == MNT_CDATA_SECTION )
					strData += GKStringMID( m_strDoc, node.nStart+9, node.nLength-12 );
				else if ( node.nNodeType == MNT_ELEMENT )
				{
					GKStringCLEAR(strData);
					break;
				}
				else if ( node.nNodeType == 0 )
				{
					if ( token.Match(m_pFilePos->m_elemstack.Current().strTagName) )
					{
						pElem->SetEndTagLen( node.nLength );
						pElem->nLength = node.nStart + node.nLength - pElem->nStart;
						m_pFilePos->m_elemstack.OutOfLevel();
					}
					else
					{
						GKStringCLEAR(strData);
					}
					break;
				}
			}
		}
		else if ( ! pElem->iElemChild )
		{
			// Quick scan for any tags inside content
			GKint32 nContentLen = pElem->ContentLen();
			GKPCSTR pszContent = &(MCD_2PCSZ(m_strDoc))[nStartContent];
			GKPCSTR pszTag = MCD_PSZCHR( pszContent, '<' );
			if ( pszTag && ((GKint32)(pszTag-pszContent) < nContentLen) )
			{
				// Concatenate all CDATA Sections and text nodes, ignore other nodes
				TokenPos token( m_strDoc, m_nDocFlags );
				token.m_nNext = nStartContent;
				NodePos node;
				while ( token.m_nNext < nStartContent + nContentLen )
				{
					token.ParseNode( node );
					if ( node.nNodeType == MNT_TEXT )
						strData += UnescapeText( &token.m_pDocText[node.nStart], node.nLength, m_nDocFlags );
					else if ( node.nNodeType == MNT_CDATA_SECTION )
						strData += GKStringMID( m_strDoc, node.nStart+9, node.nLength-12 );
				}
			}
			else // no tags
				strData = UnescapeText( &(MCD_2PCSZ(m_strDoc))[nStartContent], nContentLen, m_nDocFlags );
		}
	}
	return strData;
}

GKString CMarkup::x_GetElemContent( GKint32 iPos ) const
{
	if ( ! (m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE)) )
	{
		ElemPos* pElem = &ELEM(iPos);
		if ( iPos && pElem->ContentLen() )
			return GKStringMID( m_strDoc, pElem->StartContent(), pElem->ContentLen() );
	}
	return _U("");
}

GKbool CMarkup::x_SetElemContent( GKPCSTR szContent )
{
	GKStringCLEAR(m_strResult);
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;

	// Set data in iPos element only
	if ( ! m_iPos )
		return FALSE;

	if ( m_nNodeLength )
		return FALSE; // not an element

	// Unlink all children
	GKint32 iPos = m_iPos;
	GKint32 iPosChild = ELEM(iPos).iElemChild;
	GKbool bHadChild = (iPosChild != 0);
	while ( iPosChild )
		iPosChild = x_ReleaseSubDoc( iPosChild );
	if ( bHadChild )
		x_CheckSavedPos();

	// Parse content
	GKbool bWellFormed = TRUE;
	TokenPos token( szContent, m_nDocFlags );
	GKint32 iPosVirtual = x_GetFreePos();
	ELEM(iPosVirtual).ClearVirtualParent();
	ELEM(iPosVirtual).SetLevel( ELEM(iPos).Level() + 1 );
	iPosChild = x_ParseElem( iPosVirtual, token );
	if ( ELEM(iPosVirtual).nFlags & MNF_ILLFORMED )
		bWellFormed = FALSE;
	ELEM(iPos).nFlags = (ELEM(iPos).nFlags & ~MNF_ILLDATA) | (ELEM(iPosVirtual).nFlags & MNF_ILLDATA);

	// Prepare insert and adjust offsets
	NodePos node( MNF_WITHNOLINES|MNF_REPLACE );
	node.strMeta = szContent;
	GKint32 iPosBefore = 0;
	GKint32 nReplace = x_InsertNew( iPos, iPosBefore, node );
	
	// Adjust and link in the inserted elements
	x_Adjust( iPosChild, node.nStart );
	ELEM(iPosChild).nStart += node.nStart;
	ELEM(iPos).iElemChild = iPosChild;
	while ( iPosChild )
	{
		ELEM(iPosChild).iElemParent = iPos;
		iPosChild = ELEM(iPosChild).iElemNext;
	}
	x_ReleasePos( iPosVirtual );

	GKint32 nAdjust = node.strMeta.GetLength() - nReplace;
	x_Adjust( iPos, nAdjust, TRUE );
	ELEM(iPos).nLength += nAdjust;

	x_SetPos( m_iPosParent, m_iPos, 0 );
	return bWellFormed;
}

void CMarkup::x_DocChange( GKint32 nLeft, GKint32 nReplace, const GKString& strInsert )
{
	x_StrInsertReplace( m_strDoc, nLeft, nReplace, strInsert );
}

void CMarkup::x_Adjust( GKint32 iPos, GKint32 nShift, GKbool bAfterPos /*=FALSE*/ )
{
	// Loop through affected elements and adjust indexes
	// Algorithm:
	// 1. update children unless bAfterPos
	//    (if no children or bAfterPos is TRUE, length of iPos not affected)
	// 2. update starts of next siblings and their children
	// 3. go up until there is a next sibling of a parent and update starts
	// 4. step 2
	GKint32 iPosTop = ELEM(iPos).iElemParent;
	GKbool bPosFirst = bAfterPos; // mark as first to skip its children

	// Stop when we've reached the virtual parent (which has no tags)
	while ( ELEM(iPos).StartTagLen() )
	{
		// Were we at containing parent of affected position?
		GKbool bPosTop = FALSE;
		if ( iPos == iPosTop )
		{
			// Move iPosTop up one towards root
			iPosTop = ELEM(iPos).iElemParent;
			bPosTop = TRUE;
		}

		// Traverse to the next update position
		if ( ! bPosTop && ! bPosFirst && ELEM(iPos).iElemChild )
		{
			// Depth first
			iPos = ELEM(iPos).iElemChild;
		}
		else if ( ELEM(iPos).iElemNext )
		{
			iPos = ELEM(iPos).iElemNext;
		}
		else
		{
			// Look for next sibling of a parent of iPos
			// When going back up, parents have already been done except iPosTop
			while ( 1 )
			{
				iPos = ELEM(iPos).iElemParent;
				if ( iPos == iPosTop )
					break;
				if ( ELEM(iPos).iElemNext )
				{
					iPos = ELEM(iPos).iElemNext;
					break;
				}
			}
		}
		bPosFirst = FALSE;

		// Shift indexes at iPos
		if ( iPos != iPosTop )
			ELEM(iPos).nStart += nShift;
		else
			ELEM(iPos).nLength += nShift;
	}
}

GKint32 CMarkup::x_InsertNew( GKint32 iPosParent, GKint32& iPosRel, NodePos& node )
{
	// Parent empty tag or tags with no content?
	GKbool bEmptyParentTag = iPosParent && ELEM(iPosParent).IsEmptyElement();
	GKbool bNoContentParentTags = iPosParent && ! ELEM(iPosParent).ContentLen();
	if ( iPosRel && ! node.nLength ) // current position element?
	{
		node.nStart = ELEM(iPosRel).nStart;
		if ( ! (node.nNodeFlags & MNF_INSERT) ) // follow iPosRel
			node.nStart += ELEM(iPosRel).nLength;
	}
	else if ( bEmptyParentTag ) // parent has no separate end tag?
	{
		// Split empty parent element
		if ( ELEM(iPosParent).nFlags & MNF_NONENDED )
			node.nStart = ELEM(iPosParent).StartContent();
		else
			node.nStart = ELEM(iPosParent).StartContent() - 1;
	}
	else if ( node.nLength || (m_nDocFlags&MDF_WRITEFILE) ) // non-element node or a file mode zero length position?
	{
		if ( ! (node.nNodeFlags & MNF_INSERT) )
			node.nStart += node.nLength; // after node or file mode position
	}
	else // no current node
	{
		// Insert relative to parent's content
		if ( node.nNodeFlags & (MNF_INSERT|MNF_REPLACE) )
			node.nStart = ELEM(iPosParent).StartContent(); // beginning of parent's content
		else // in front of parent's end tag
			node.nStart = ELEM(iPosParent).StartAfter() - ELEM(iPosParent).EndTagLen();
	}

	// Go up to start of next node, unless its splitting an empty element
	if ( ! (node.nNodeFlags&(MNF_WITHNOLINES|MNF_REPLACE)) && ! bEmptyParentTag )
	{
		TokenPos token( m_strDoc, m_nDocFlags );
		node.nStart = token.WhitespaceToTag( node.nStart );
	}

	// Is insert relative to element position? (i.e. not other kind of node)
	if ( ! node.nLength )
	{
		// Modify iPosRel to reflect position before
		if ( iPosRel )
		{
			if ( node.nNodeFlags & MNF_INSERT )
			{
				if ( ! (ELEM(iPosRel).nFlags & MNF_FIRST) )
					iPosRel = ELEM(iPosRel).iElemPrev;
				else
					iPosRel = 0;
			}
		}
		else if ( ! (node.nNodeFlags & MNF_INSERT) )
		{
			// If parent has a child, add after last child
			if ( ELEM(iPosParent).iElemChild )
				iPosRel = ELEM(ELEM(iPosParent).iElemChild).iElemPrev;
		}
	}

	// Get node length (needed for x_AddNode and x_AddSubDoc in file write mode)
	node.nLength = node.strMeta.GetLength();

	// Prepare end of lines
	if ( (! (node.nNodeFlags & MNF_WITHNOLINES)) && (bEmptyParentTag || bNoContentParentTags) )
		node.nStart += MCD_EOLLEN;
	if ( ! (node.nNodeFlags & MNF_WITHNOLINES) )
		node.strMeta += MCD_EOL;

	// Calculate insert offset and replace length
	GKint32 nReplace = 0;
	GKint32 nInsertAt = node.nStart;
	if ( bEmptyParentTag )
	{
		GKString strTagName = x_GetTagName( iPosParent );
		GKString strFormat;
		if ( node.nNodeFlags & MNF_WITHNOLINES )
			strFormat = _U(">");
		else
			strFormat = _U(">") MCD_EOL;
		strFormat += node.strMeta;
		strFormat += _U("</");
		strFormat += strTagName;
		node.strMeta = strFormat;
		if ( ELEM(iPosParent).nFlags & MNF_NONENDED )
		{
			nInsertAt = ELEM(iPosParent).StartAfter() - 1;
			nReplace = 0;
			ELEM(iPosParent).nFlags ^= MNF_NONENDED;
		}
		else
		{
			nInsertAt = ELEM(iPosParent).StartAfter() - 2;
			nReplace = 1;
			ELEM(iPosParent).AdjustStartTagLen( -1 );
		}
		ELEM(iPosParent).SetEndTagLen( 3 + strTagName.GetLength() );
	}
	else
	{
		if ( node.nNodeFlags & MNF_REPLACE )
		{
			nInsertAt = ELEM(iPosParent).StartContent();
			nReplace = ELEM(iPosParent).ContentLen();
		}
		else if ( bNoContentParentTags )
		{
			node.strMeta = MCD_EOL + node.strMeta;
			nInsertAt = ELEM(iPosParent).StartContent();
		}
	}
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		// Check if buffer is full
		GKint32 nNewDocLength = m_strDoc.GetLength() + node.strMeta.GetLength() - nReplace;
		GKint32 nFlushTo = node.nStart;
		GKStringCLEAR( m_strResult );
		if ( bEmptyParentTag )
			nFlushTo = ELEM(iPosParent).nStart;
		if ( nFlushTo && nNewDocLength > m_pFilePos->m_nBlockSizeBasis )
		{
			GKint32 nDocCapacity = GKStringCAPACITY(m_strDoc);
			if ( nNewDocLength > nDocCapacity )
			{
				if ( bEmptyParentTag )
					ELEM(iPosParent).nStart = 0;
				node.nStart -= nFlushTo;
				nInsertAt -= nFlushTo;
				m_pFilePos->FileFlush( m_strDoc, nFlushTo );
				m_strResult = m_pFilePos->m_strIOResult;
			}
		}
	}
	x_DocChange( nInsertAt, nReplace, node.strMeta );
	return nReplace;
}

GKbool CMarkup::x_AddElem( GKPCSTR pName, GKint32 nValue, GKint32 nFlags )
{
	// Convert integer to string
	GKuchar szVal[25];
	MCD_SPRINTF( MCD_SSZ(szVal), _U("%d"), nValue );
	return x_AddElem( pName, szVal, nFlags );
}

GKbool CMarkup::x_AddElem( GKPCSTR pName, GKPCSTR pValue, GKint32 nFlags )
{
	if ( m_nDocFlags & MDF_READFILE )
		return FALSE;
	if ( nFlags & MNF_CHILD )
	{
		// Adding a child element under main position
		if ( ! m_iPos || (m_nDocFlags & MDF_WRITEFILE) )
			return FALSE;
	}

	// Cannot have data in non-ended element
	if ( (nFlags&MNF_WITHNOEND) && pValue && pValue[0] )
		return FALSE;

	// Node and element structures
	NodePos node( nFlags );
	GKint32 iPosParent = 0, iPosBefore = 0;
	GKint32 iPos = x_GetFreePos();
	ElemPos* pElem = &ELEM(iPos);

	// Locate where to add element relative to current node
	if ( nFlags & MNF_CHILD )
	{
		iPosParent = m_iPos;
		iPosBefore = m_iPosChild;
	}
	else
	{
		iPosParent = m_iPosParent;
		iPosBefore = m_iPos;
		node.nStart = m_nNodeOffset;
		node.nLength = m_nNodeLength;
	}

	// Create string for insert
	// If no pValue is specified, an empty element is created
	// i.e. either <NAME>value</NAME> or <NAME/>
	//
	GKint32 nLenName = GKSTRLEN(pName);
	if ( ! pValue || ! pValue[0] )
	{
		// <NAME/> empty element
		MCD_BLDRESERVE( node.strMeta, nLenName + 4 );
		MCD_BLDAPPEND1( node.strMeta, '<' );
		MCD_BLDAPPENDN( node.strMeta, pName, nLenName );
		if ( nFlags & MNF_WITHNOEND )
		{
			MCD_BLDAPPEND1( node.strMeta, '>' );
		}
		else
		{
			if ( nFlags & MNF_WITHXHTMLSPACE )
			{
				MCD_BLDAPPENDN( node.strMeta, _U(" />"), 3 );
			}
			else
			{
				MCD_BLDAPPENDN( node.strMeta, _U("/>"), 2 );
			}
		}
		MCD_BLDRELEASE( node.strMeta );
		pElem->nLength = node.strMeta .GetLength();
		pElem->SetStartTagLen( pElem->nLength );
		pElem->SetEndTagLen( 0 );
	}
	else
	{
		// <NAME>value</NAME>
		GKString strValue;
		if ( nFlags & MNF_WITHCDATA )
			strValue = x_EncodeCDATASection( pValue );
		else
			strValue = EscapeText( pValue, nFlags );
		GKint32 nLenValue = strValue.GetLength();
		pElem->nLength = nLenName * 2 + nLenValue + 5;
		MCD_BLDRESERVE( node.strMeta, pElem->nLength );
		MCD_BLDAPPEND1( node.strMeta, '<' );
		MCD_BLDAPPENDN( node.strMeta, pName, nLenName );
		MCD_BLDAPPEND1( node.strMeta, '>' );
		MCD_BLDAPPENDN( node.strMeta, MCD_2PCSZ(strValue), nLenValue );
		MCD_BLDAPPENDN( node.strMeta, _U("</"), 2 );
		MCD_BLDAPPENDN( node.strMeta, pName, nLenName );
		MCD_BLDAPPEND1( node.strMeta, '>' );
		MCD_BLDRELEASE( node.strMeta );
		pElem->SetEndTagLen( nLenName + 3 );
		pElem->SetStartTagLen( nLenName + 2 );
	}

	// Insert
	GKint32 nReplace = x_InsertNew( iPosParent, iPosBefore, node );
	pElem->nStart = node.nStart;
	pElem->iElemChild = 0;
	if ( nFlags & MNF_WITHNOEND )
		pElem->nFlags = MNF_NONENDED;
	else
		pElem->nFlags = 0;
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		iPosParent = x_UnlinkPrevElem( iPosParent, iPosBefore, iPos );
		TokenPos token( m_strDoc, m_nDocFlags );
		token.m_nL = pElem->nStart + 1;
		token.m_nR = pElem->nStart + nLenName;
		m_pFilePos->m_elemstack.PushTagAndCount( token );
	}
	else
	{
		x_LinkElem( iPosParent, iPosBefore, iPos );
		x_Adjust( iPos, node.strMeta.GetLength() - nReplace );
	}
	if ( nFlags & MNF_CHILD )
		x_SetPos( m_iPosParent, iPosParent, iPos );
	else
		x_SetPos( iPosParent, iPos, 0 );
	return TRUE;
}

GKString CMarkup::x_GetSubDoc( GKint32 iPos )
{
	if ( iPos && ! (m_nDocFlags&MDF_WRITEFILE) )
	{
		if ( ! (m_nDocFlags&MDF_READFILE) )
		{
			TokenPos token( m_strDoc, m_nDocFlags );
			token.WhitespaceToTag( ELEM(iPos).StartAfter() );
			token.m_nL = ELEM(iPos).nStart;
			return token.GetTokenText();
		}
	}
	return _U("");
}

GKbool CMarkup::x_AddSubDoc( GKPCSTR pSubDoc, GKint32 nFlags )
{
	if ( m_nDocFlags & MDF_READFILE || ((nFlags & MNF_CHILD) && (m_nDocFlags & MDF_WRITEFILE)) )
		return FALSE;

	GKStringCLEAR(m_strResult);
	NodePos node( nFlags );
	GKint32 iPosParent, iPosBefore;
	if ( nFlags & MNF_CHILD )
	{
		// Add a subdocument under main position, before or after child
		if ( ! m_iPos )
			return FALSE;
		iPosParent = m_iPos;
		iPosBefore = m_iPosChild;
	}
	else
	{
		// Add a subdocument under parent position, before or after main
		iPosParent = m_iPosParent;
		iPosBefore = m_iPos;
		node.nStart = m_nNodeOffset;
		node.nLength = m_nNodeLength;
	}

	// Parse subdocument, generating indexes based on the subdocument string to be offset later
	GKbool bWellFormed = TRUE;
	TokenPos token( pSubDoc, m_nDocFlags );
	GKint32 iPosVirtual = x_GetFreePos();
	ELEM(iPosVirtual).ClearVirtualParent();
	ELEM(iPosVirtual).SetLevel( ELEM(iPosParent).Level() + 1 );
	GKint32 iPos = x_ParseElem( iPosVirtual, token );
	if ( (!iPos) || ELEM(iPosVirtual).nFlags & MNF_ILLFORMED )
		bWellFormed = FALSE;
	if ( ELEM(iPosVirtual).nFlags & MNF_ILLDATA )
		ELEM(iPosParent).nFlags |= MNF_ILLDATA;

	// File write mode handling
	GKbool bBypassSubDoc = FALSE;
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		// Current position will bypass subdoc unless well-formed single element
		if ( (! bWellFormed) || ELEM(iPos).iElemChild || ELEM(iPos).iElemNext )
			bBypassSubDoc = TRUE;

		// Count tag names of top level elements (usually one) in given markup
		GKint32 iPosTop = iPos;
		while ( iPosTop )
		{
			token.m_nNext = ELEM(iPosTop).nStart + 1;
			token.FindName();
			m_pFilePos->m_elemstack.PushTagAndCount( token );
			iPosTop = ELEM(iPosTop).iElemNext;
		}
	}

	// Extract subdocument without leading/trailing nodes
	GKint32 nExtractStart = 0;
	GKint32 iPosLast = ELEM(iPos).iElemPrev;
	if ( bWellFormed )
	{
		nExtractStart = ELEM(iPos).nStart;
		GKint32 nExtractLength = ELEM(iPos).nLength;
		if ( iPos != iPosLast )
		{
			nExtractLength = ELEM(iPosLast).nStart - nExtractStart + ELEM(iPosLast).nLength;
			bWellFormed = FALSE; // treat as subdoc here, but return not well-formed
		}
		GKStringASSIGN(node.strMeta,&pSubDoc[nExtractStart],nExtractLength);
	}
	else
	{
		node.strMeta = pSubDoc;
		node.nNodeFlags |= MNF_WITHNOLINES;
	}

	// Insert
	GKint32 nReplace = x_InsertNew( iPosParent, iPosBefore, node );

	// Clean up indexes
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		if ( bBypassSubDoc )
		{
			// Release indexes used in parsing the subdocument
			m_iPosParent = x_UnlinkPrevElem( iPosParent, iPosBefore, 0 );
			m_iPosFree = 1;
			m_iPosDeleted = 0;
			m_iPos = 0;
			m_nNodeOffset = node.nStart + node.nLength;
			m_nNodeLength = 0;
			m_nNodeType = 0;
			MARKUP_SETDEBUGSTATE;
			return bWellFormed;
		}
		else // single element added
		{
			m_iPos = iPos;
			ElemPos* pElem = &ELEM(iPos);
			pElem->nStart = node.nStart;
			m_iPosParent = x_UnlinkPrevElem( iPosParent, iPosBefore, iPos );
			x_ReleasePos( iPosVirtual );
		}
	}
	else
	{
		// Adjust and link in the inserted elements
		// iPosVirtual will stop it from affecting rest of document
		GKint32 nAdjust = node.nStart - nExtractStart;
		if ( iPos && nAdjust )
		{
			x_Adjust( iPos, nAdjust );
			ELEM(iPos).nStart += nAdjust;
		}
		GKint32 iPosChild = iPos;
		while ( iPosChild )
		{
			GKint32 iPosNext = ELEM(iPosChild).iElemNext;
			x_LinkElem( iPosParent, iPosBefore, iPosChild );
			iPosBefore = iPosChild;
			iPosChild = iPosNext;
		}
		x_ReleasePos( iPosVirtual );

		// Now adjust remainder of document
		x_Adjust( iPosLast, node.strMeta.GetLength() - nReplace, TRUE );
	}

	// Set position to top element of subdocument
	if ( nFlags & MNF_CHILD )
		x_SetPos( m_iPosParent, iPosParent, iPos );
	else // Main
		x_SetPos( m_iPosParent, iPos, 0 );
	return bWellFormed;
}

GKint32 CMarkup::x_RemoveElem( GKint32 iPos )
{
	// Determine whether any whitespace up to next tag
	TokenPos token( m_strDoc, m_nDocFlags );
	GKint32 nAfterEnd = token.WhitespaceToTag( ELEM(iPos).StartAfter() );

	// Remove from document, adjust affected indexes, and unlink
	GKint32 nLen = nAfterEnd - ELEM(iPos).nStart;
	x_DocChange( ELEM(iPos).nStart, nLen, GKString() );
	x_Adjust( iPos, - nLen, TRUE );
	GKint32 iPosPrev = x_UnlinkElem( iPos );
	x_CheckSavedPos();
	return iPosPrev; // new position
}

void CMarkup::x_LinkElem( GKint32 iPosParent, GKint32 iPosBefore, GKint32 iPos )
{
	// Update links between elements and initialize nFlags
	ElemPos* pElem = &ELEM(iPos);
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		// In file write mode, only keep virtual parent 0 plus one element 
		if ( iPosParent )
			x_ReleasePos( iPosParent );
		else if ( iPosBefore )
			x_ReleasePos( iPosBefore );
		iPosParent = 0;
		ELEM(iPosParent).iElemChild = iPos;
		pElem->iElemParent = iPosParent;
		pElem->iElemPrev = iPos;
		pElem->iElemNext = 0;
		pElem->nFlags |= MNF_FIRST;
	}
	else
	{
		pElem->iElemParent = iPosParent;
		if ( iPosBefore )
		{
			// Link in after iPosBefore
			pElem->nFlags &= ~MNF_FIRST;
			pElem->iElemNext = ELEM(iPosBefore).iElemNext;
			if ( pElem->iElemNext )
				ELEM(pElem->iElemNext).iElemPrev = iPos;
			else
				ELEM(ELEM(iPosParent).iElemChild).iElemPrev = iPos;
			ELEM(iPosBefore).iElemNext = iPos;
			pElem->iElemPrev = iPosBefore;
		}
		else
		{
			// Link in as first child
			pElem->nFlags |= MNF_FIRST;
			if ( ELEM(iPosParent).iElemChild )
			{
				pElem->iElemNext = ELEM(iPosParent).iElemChild;
				pElem->iElemPrev = ELEM(pElem->iElemNext).iElemPrev;
				ELEM(pElem->iElemNext).iElemPrev = iPos;
				ELEM(pElem->iElemNext).nFlags ^= MNF_FIRST;
			}
			else
			{
				pElem->iElemNext = 0;
				pElem->iElemPrev = iPos;
			}
			ELEM(iPosParent).iElemChild = iPos;
		}
		if ( iPosParent )
			pElem->SetLevel( ELEM(iPosParent).Level() + 1 );
	}
}

GKint32 CMarkup::x_UnlinkElem( GKint32 iPos )
{
	// Fix links to remove element and mark as deleted
	// return previous position or zero if none
	ElemPos* pElem = &ELEM(iPos);

	// Find previous sibling and bypass removed element
	GKint32 iPosPrev = 0;
	if ( pElem->nFlags & MNF_FIRST )
	{
		if ( pElem->iElemNext ) // set next as first child
		{
			ELEM(pElem->iElemParent).iElemChild = pElem->iElemNext;
			ELEM(pElem->iElemNext).iElemPrev = pElem->iElemPrev;
			ELEM(pElem->iElemNext).nFlags |= MNF_FIRST;
		}
		else // no children remaining
			ELEM(pElem->iElemParent).iElemChild = 0;
	}
	else
	{
		iPosPrev = pElem->iElemPrev;
		ELEM(iPosPrev).iElemNext = pElem->iElemNext;
		if ( pElem->iElemNext )
			ELEM(pElem->iElemNext).iElemPrev = iPosPrev;
		else
			ELEM(ELEM(pElem->iElemParent).iElemChild).iElemPrev = iPosPrev;
	}
	x_ReleaseSubDoc( iPos );
	return iPosPrev;
}

GKint32 CMarkup::x_UnlinkPrevElem( GKint32 iPosParent, GKint32 iPosBefore, GKint32 iPos )
{
	// In file write mode, only keep virtual parent 0 plus one element if currently at element
	if ( iPosParent )
	{
		x_ReleasePos( iPosParent );
		iPosParent = 0;
	}
	else if ( iPosBefore )
		x_ReleasePos( iPosBefore );
	ELEM(iPosParent).iElemChild = iPos;
	ELEM(iPosParent).nLength = m_strDoc.GetLength();
	if ( iPos )
	{
		ElemPos* pElem = &ELEM(iPos);
		pElem->iElemParent = iPosParent;
		pElem->iElemPrev = iPos;
		pElem->iElemNext = 0;
		pElem->nFlags |= MNF_FIRST;
	}
	return iPosParent;
}

GKint32 CMarkup::x_ReleasePos( GKint32 iPos )
{
	GKint32 iPosNext = ELEM(iPos).iElemNext;
	ELEM(iPos).iElemNext = m_iPosDeleted;
	ELEM(iPos).nFlags = MNF_DELETED;
	m_iPosDeleted = iPos;
	return iPosNext;
}

GKint32 CMarkup::x_ReleaseSubDoc( GKint32 iPos )
{
	// Mark position structures as deleted by depth first traversal
	// Tricky because iElemNext used in traversal is overwritten for linked list of deleted
	// Return value is what iElemNext was before being overwritten
	//
	GKint32 iPosNext = 0, iPosTop = iPos;
	while ( 1 )
	{
		if ( ELEM(iPos).iElemChild )
			iPos = ELEM(iPos).iElemChild;
		else
		{
			while ( 1 )
			{
				iPosNext = x_ReleasePos( iPos );
				if ( iPosNext || iPos == iPosTop )
					break;
				iPos = ELEM(iPos).iElemParent;
			}
			if ( iPos == iPosTop )
				break;
			iPos = iPosNext;
		}
	}
	return iPosNext;
}

void CMarkup::x_CheckSavedPos()
{
	// Remove any saved positions now pointing to deleted elements
	// Must be done as part of element removal before position reassigned
	if ( m_pSavedPosMaps->m_pMaps )
	{
		GKint32 nMap = 0;
		while ( m_pSavedPosMaps->m_pMaps[nMap] )
		{
			SavedPosMap* pMap = m_pSavedPosMaps->m_pMaps[nMap];
			for ( GKint32 nSlot = 0; nSlot < pMap->nMapSize; ++nSlot )
			{
				SavedPos* pSavedPos = pMap->pTable[nSlot];
				if ( pSavedPos )
				{
					GKint32 nOffset = 0;
					GKint32 nSavedPosCount = 0;
					while ( 1 )
					{
						if ( pSavedPos[nOffset].nSavedPosFlags & SavedPos::SPM_USED )
						{
							GKint32 iPos = pSavedPos[nOffset].iPos;
							if ( ! (ELEM(iPos).nFlags & MNF_DELETED) )
							{
								if ( nSavedPosCount < nOffset )
								{
									pSavedPos[nSavedPosCount] = pSavedPos[nOffset];
									pSavedPos[nSavedPosCount].nSavedPosFlags &= ~SavedPos::SPM_LAST;
								}
								++nSavedPosCount;
							}
						}
						if ( pSavedPos[nOffset].nSavedPosFlags & SavedPos::SPM_LAST )
						{
							while ( nSavedPosCount <= nOffset )
								pSavedPos[nSavedPosCount++].nSavedPosFlags &= ~SavedPos::SPM_USED;
							break;
						}
						++nOffset;
					}
				}
			}
			++nMap;
		}
	}
}

void CMarkup::x_AdjustForNode( GKint32 iPosParent, GKint32 iPos, GKint32 nShift )
{
	// Adjust affected indexes
	GKbool bAfterPos = TRUE;
	if ( ! iPos )
	{
		// Change happened before or at first element under iPosParent
		// If there are any children of iPosParent, adjust from there
		// otherwise start at parent and adjust from there
		iPos = ELEM(iPosParent).iElemChild;
		if ( iPos )
		{
			ELEM(iPos).nStart += nShift;
			bAfterPos = FALSE;
		}
		else
		{
			iPos = iPosParent;
			ELEM(iPos).nLength += nShift;
		}
	}
	x_Adjust( iPos, nShift, bAfterPos );
}

GKbool CMarkup::x_AddNode( GKint32 nNodeType, GKPCSTR pText, GKint32 nNodeFlags )
{
	if ( m_nDocFlags & MDF_READFILE )
		return FALSE;

	// Comments, DTDs, and processing instructions are followed by CRLF
	// Other nodes are usually concerned with mixed content, so no CRLF
	if ( ! (nNodeType & (MNT_PROCESSING_INSTRUCTION|MNT_COMMENT|MNT_DOCUMENT_TYPE)) )
		nNodeFlags |= MNF_WITHNOLINES;

	// Add node of nNodeType after current node position
	NodePos node( nNodeFlags );
	if ( ! x_CreateNode(node.strMeta, nNodeType, pText) )
		return FALSE;

	// Insert the new node relative to current node
	node.nStart = m_nNodeOffset;
	node.nLength = m_nNodeLength;
	node.nNodeType = nNodeType;
	GKint32 iPosBefore = m_iPos;
	GKint32 nReplace = x_InsertNew( m_iPosParent, iPosBefore, node );

	// If its a new element, create an ElemPos
	GKint32 iPos = iPosBefore;
	ElemPos* pElem = NULL;
	if ( nNodeType == MNT_ELEMENT )
	{
		// Set indexes
		iPos = x_GetFreePos();
		pElem = &ELEM(iPos);
		pElem->nStart = node.nStart;
		pElem->SetStartTagLen( node.nLength );
		pElem->SetEndTagLen( 0 );
		pElem->nLength = node.nLength;
		node.nStart = 0;
		node.nLength = 0;
		pElem->iElemChild = 0;
		pElem->nFlags = 0;
		x_LinkElem( m_iPosParent, iPosBefore, iPos );
	}
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		m_iPosParent = x_UnlinkPrevElem( m_iPosParent, iPosBefore, iPos );
		if ( nNodeType == MNT_ELEMENT )
		{
			TokenPos token( m_strDoc, m_nDocFlags );
			token.m_nL = pElem->nStart + 1;
			token.m_nR = pElem->nStart + pElem->nLength - 3;
			m_pFilePos->m_elemstack.PushTagAndCount( token );
		}
	}
	else // need to adjust element positions after iPos
		x_AdjustForNode( m_iPosParent, iPos, node.strMeta.GetLength() - nReplace );

	// Store current position
	m_iPos = iPos;
	m_iPosChild = 0;
	m_nNodeOffset = node.nStart;
	m_nNodeLength = node.nLength;
	m_nNodeType = nNodeType;
	MARKUP_SETDEBUGSTATE;
	return TRUE;
}

void CMarkup::x_RemoveNode( GKint32 iPosParent, GKint32& iPos, GKint32& nNodeType, GKint32& nNodeOffset, GKint32& nNodeLength )
{
	GKint32 iPosPrev = iPos;

	// Removing an element?
	if ( nNodeType == MNT_ELEMENT )
	{
		nNodeOffset = ELEM(iPos).nStart;
		nNodeLength = ELEM(iPos).nLength;
		iPosPrev = x_UnlinkElem( iPos );
		x_CheckSavedPos();
	}

	// Find previous node type, offset and length
	GKint32 nPrevOffset = 0;
	if ( iPosPrev )
		nPrevOffset = ELEM(iPosPrev).StartAfter();
	else if ( iPosParent )
		nPrevOffset = ELEM(iPosParent).StartContent();
	TokenPos token( m_strDoc, m_nDocFlags );
	NodePos node;
	token.m_nNext = nPrevOffset;
	GKint32 nPrevType = 0;
	while ( token.m_nNext < nNodeOffset )
	{
		nPrevOffset = token.m_nNext;
		nPrevType = token.ParseNode( node );
	}
	GKint32 nPrevLength = nNodeOffset - nPrevOffset;
	if ( ! nPrevLength )
	{
		// Previous node is iPosPrev element
		nPrevOffset = 0;
		if ( iPosPrev )
			nPrevType = MNT_ELEMENT;
	}

	// Remove node from document
 	x_DocChange( nNodeOffset, nNodeLength, GKString() );
	x_AdjustForNode( iPosParent, iPosPrev, - nNodeLength );

	// Was removed node a lone end tag?
	if ( nNodeType == MNT_LONE_END_TAG )
	{
		// See if we can unset parent MNF_ILLDATA flag
		token.m_nNext = ELEM(iPosParent).StartContent();
		GKint32 nEndOfContent = token.m_nNext + ELEM(iPosParent).ContentLen();
		GKint32 iPosChild = ELEM(iPosParent).iElemChild;
		while ( token.m_nNext < nEndOfContent )
		{
			if ( token.ParseNode(node) <= 0 )
				break;
			if ( node.nNodeType == MNT_ELEMENT )
			{
				token.m_nNext = ELEM(iPosChild).StartAfter();
				iPosChild = ELEM(iPosChild).iElemNext;
			}
		}
		if ( token.m_nNext == nEndOfContent )
			ELEM(iPosParent).nFlags &= ~MNF_ILLDATA;
	}

	nNodeType = nPrevType;
	nNodeOffset = nPrevOffset;
	nNodeLength = nPrevLength;
	iPos = iPosPrev;
}
