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
#include "Base/GKTextEncoding.h"

#define x_ATTRIBQUOTE '\"' // can be double or single quote

// End of line choices: none, return, newline, or CRLF
#if defined(MARKUP_EOL_NONE)
#define MARKUP_EOL _U("")
#elif defined(MARKUP_EOL_RETURN) // rare; only used on some old operating systems
#define MARKUP_EOL _U("\r")
#elif defined(MARKUP_EOL_NEWLINE) // Unix standard
#define MARKUP_EOL _U("\n")
#elif defined(MARKUP_EOL_CRLF) || defined(MARKUP_WINDOWS) // Windows standard
#define MARKUP_EOL _U("\r\n")
#else // not Windows and not otherwise specified
#define MARKUP_EOL _U("\n")
#endif // not Windows and not otherwise specified
#define MARKUP_EOLLEN (sizeof(MARKUP_EOL)/sizeof(GKuchar)-1) // string length of MARKUP_EOL

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

using namespace GKBASE;
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

GKint32 x_Hash( const GKuchar* p, GKint32 nSize )
{
	GKuint32 n=0;
	while (*p)
		n += (GKuint32)(*p++);
	return n % nSize;
}

GKString x_IntToStr( GKint32 n )
{
	GKuchar sz[25];
	GKSPRINTF((GKuchar*)sz,_U("%d"),n);
	GKString s=sz;
	return s;
}

GKint32 x_StrNCmp( const GKuchar* p1, const GKuchar* p2, GKint32 n, GKint32 bIgnoreCase = 0 )
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
			else if ( (GKuint32)c1 > 127 )
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

// Encoding names
// This is a precompiled ASCII hash table for speed and minimum memory requirement
// Each entry consists of a 2 digit name length, 5 digit code page, and the encoding name
// Each table slot can have multiple entries, table size 155 was chosen for even distribution
//
const GKuchar* EncodingNameTable[155] =
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
	GKuint32 nStartTagLen : 22; // 4MB limit for start tag
	GKuint32 nEndTagLen : 10; // 1K limit for end tag
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
	TokenPos( const GKuchar* sz, GKint32 n, FilePos* p=NULL ) { Clear(); m_pDocText=sz; m_nTokenFlags=n; m_pReaderFilePos=p; };
	TokenPos( const GKString& str, GKint32 n, FilePos* p=NULL ) { Clear(); const GKuchar* sz= str.Cstr();m_pDocText=sz; m_nTokenFlags=n; m_pReaderFilePos=p; };
	void Clear() { m_nL=0; m_nR=-1; m_nNext=0; };
	GKint32 Length() const { return m_nR - m_nL + 1; };
	const GKuchar* GetTokenPtr() const { return &m_pDocText[m_nL]; };
	GKString GetTokenText() const { return GKString( GetTokenPtr(), Length() ); };
	GKuchar NextChar() { m_nNext += (GKint32)GKSTRLEN(&m_pDocText[m_nNext]); return m_pDocText[m_nNext]; };
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
	GKbool Match( const GKuchar* szName )
	{
		GKint32 nLen = Length();
		return ( (x_StrNCmp( GetTokenPtr(), szName, nLen, m_nTokenFlags & CMarkup::MDF_IGNORECASE ) == 0)
			&& ( szName[nLen] == '\0' || x_ISENDPATHWORD(szName[nLen]) ) );
	};

	GKbool Match( const GKString& str )
	{
		return Match(str.Cstr());
	};
	
	GKbool FindAttrib( const GKuchar* pAttrib, GKint32 n = 0, GKString* pstrAttrib = NULL );
	GKint32 ParseNode( NodePos& node );
	GKint32 m_nL;
	GKint32 m_nR;
	GKint32 m_nNext;
	const GKuchar* m_pDocText;
	GKint32 m_nTokenFlags;
	GKint32 m_nPreSpaceStart;
	GKint32 m_nPreSpaceLength;
	FilePos* m_pReaderFilePos;
};

GKbool TokenPos::FindAttrib( const GKuchar* pAttrib, GKint32 n/*=0*/, GKString* pstrAttrib/*=NULL*/ )
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
	void SetTagName( const GKuchar* pName, GKint32 n ) { strTagName.Assign(pName, n); };
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
	void Push( const GKuchar* pName, GKint32 n ) { ++iUsed; if (iUsed==nSize) Alloc(nSize*2); pL[iUsed].SetTagName(pName,n); pL[iUsed].iParent=iPar; iTop=iUsed; };
	void IntoLevel() { iPar = iTop; ++nLevel; };
	void OutOfLevel() { if (iPar!=iTop) Pop(); iPar = pL[iTop].iParent; --nLevel; };
	void PushIntoLevel( const GKuchar* pName, GKint32 n ) { ++iTop; if (iTop==nSize) Alloc(nSize*2); pL[iTop].SetTagName(pName,n); };
	void PopOutOfLevel() { --iTop; };
	void Pop() { iTop = iPar; while (iUsed && pL[iUsed].iParent==iPar) { if (pL[iUsed].nSlot!=-1) Unslot(pL[iUsed]); --iUsed; } };
	void Slot( GKint32 n ) { pL[iUsed].nSlot=n; GKint32 i=anTable[n]; anTable[n]=iUsed; pL[iUsed].iSlotNext=i; if (i) pL[i].iSlotPrev=iUsed; };
	void Unslot( TagPos& lp ) { GKint32 n=lp.iSlotNext,p=lp.iSlotPrev; if (n) pL[n].iSlotPrev=p; if (p) pL[p].iSlotNext=n; else anTable[lp.nSlot]=n; };
	static GKint32 CalcSlot( const GKuchar* pName, GKint32 n, GKbool bIC );
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

GKint32 ElemStack::CalcSlot( const GKuchar* pName, GKint32 n, GKbool bIC )
{
	// If bIC (ASCII ignore case) then return an ASCII case insensitive hash
	GKuint32 nHash = 0;
	const GKuchar* pEnd = pName + n;
	while ( pName != pEnd )
	{
		nHash += (GKuint32)(*pName);
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
	const GKuchar* pTagName = token.GetTokenPtr();
	if ( iTop != iPar )
	{
		// See if tag name is already used, first try previous sibling (almost always)
		iNext = iTop;
		if ( token.Match(Current().strTagName.Cstr()) )
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
				if ( tag.iParent == iPar && token.Match(tag.strTagName.Cstr()) )
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
			(Current().strTagName).Assign(pTagName, token.Length());
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
	GKbool FileOpen( const GKuchar* szFileName );
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
	GKulong m_nFileByteLen;
	GKulong m_nFileByteOffset;
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

struct BomTableStruct { const char* pszBom; GKint32 nBomLen; const GKuchar* pszBomEnc; GKint32 nBomFlag; } BomTable[] =
{
	{ "\xef\xbb\xbf", 3, _U("UTF-8"), CMarkup::MDF_UTF8PREAMBLE },
	{ "\xff\xfe", 2, _U("UTF-16LE"), CMarkup::MDF_UTF16LEFILE },
	{ "\xfe\xff", 2, _U("UTF-16BE"), CMarkup::MDF_UTF16BEFILE },
	{ NULL,0,NULL,0 }
};

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
	if ( ((m_nDocFlags & CMarkup::MDF_READFILE) && m_nFileByteOffset < m_nOpFileByteLen + 5 )
			|| ((m_nDocFlags & CMarkup::MDF_WRITEFILE) && m_nFileByteOffset < 5) )
		return TRUE;
	return FALSE;
}

bool FilePos::FileErrorAddResult()
{
	// strerror has difficulties cross-platform
	// VC++ leaves MCD_STRERROR undefined and uses FormatMessage
	// Non-VC++ use strerror (even for MARKUP_WCHAR and convert)
	// additional notes:
	// _WIN32_WCE (Windows CE) has no strerror (Embedded VC++ uses FormatMessage) 
	// _MSC_VER >= 1310 (VC++ 2003/7.1) has _wcserror (but not used)
	//
	return FALSE;
}

GKbool FilePos::FileOpen( const GKuchar* szFileName )
{
	m_strIOResult.Empty();

	// Open file
	const GKuchar* pMode = _U("rb");
	if ( m_nDocFlags & CMarkup::MDF_APPENDFILE )
		pMode = _U("ab");
	else if ( m_nDocFlags & CMarkup::MDF_WRITEFILE )
		pMode = _U("wb");
	m_fp = NULL;
	 m_fp = GKFOPEN( szFileName, pMode );
	if ( ! m_fp )
		return FileErrorAddResult();

	// Prepare file
	GKbool bSuccess = TRUE;
	GKint32 nBomLen = 0;
	m_nFileCharUnitSize = 1; // unless UTF-16 BOM
	if ( m_nDocFlags & CMarkup::MDF_READFILE )
	{
		// Get file length
		GKFSEEK( m_fp, 0, SEEK_END );
		m_nFileByteLen = GKFTELL( m_fp );
		GKFSEEK( m_fp, 0, SEEK_SET );

		// Read the top of the file to check BOM and encoding
		GKint32 nReadTop = 1024;
		if ( m_nFileByteLen < nReadTop )
			nReadTop = (GKint32)m_nFileByteLen;
		if ( nReadTop )
		{
			char* pFileTop = new char[nReadTop];
			if ( nReadTop )
				bSuccess = ( GKFREAD( pFileTop, nReadTop, 1, m_fp ) == 1 );
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
					GKFSEEK( m_fp, nBomLen, SEEK_SET );

				// Encoding check
				if ( ! nBomLen )
				{
					GKString strDeclCheck;
					strDeclCheck.Assign(pFileTop, nReadTop);
					m_strEncoding = CMarkup::GetDeclaredEncoding( strDeclCheck.Cstr() );
				}
				// Assume markup files starting with < sign are UTF-8 if otherwise unknown
				if ( m_strEncoding.IsEmpty() && pFileTop[0] == '<' )
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
			GKFSEEK( m_fp, 0, SEEK_END );
			m_nFileByteLen = GKFTELL( m_fp );
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
					bSuccess = ( GKFWRITE(BomTable[nBomCheck].pszBom,nBomLen,1,m_fp) == 1 );
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
		m_nFileByteOffset = (GKulong)nBomLen;
	
	return bSuccess;
}

GKbool FilePos::FileRead( void* pBuffer )
{
	GKbool bSuccess = ( GKFREAD( pBuffer,m_nOpFileByteLen,1,m_fp) == 1 );
	m_nOpFileTextLen = m_nOpFileByteLen / m_nFileCharUnitSize;
	if ( bSuccess )
	{
		m_nFileByteOffset += m_nOpFileByteLen;

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
			m_nOpFileTextLen -= nNullCount;
		}

		// Big endian/little endian conversion
		if ( m_nFileCharUnitSize > 1 && x_EndianSwapRequired(m_nDocFlags) )
		{
			x_EndianSwapUTF16( (unsigned short*)pBuffer, m_nOpFileTextLen );
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
	//GKCOMPILEMSG ss
#pragma message("´¦ÀíTextEncoding~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
// 	TextEncoding textencoding( m_strEncoding, pBuffer, m_nOpFileTextLen );
// 	if ( ! textencoding.FindRaggedEnd(nTruncBeforeBytes) )
// 	{
// 		// Input must be garbled? decoding error before potentially ragged end, add error result and continue
// 		GKString strEncoding = m_strEncoding;
// 		if ( strEncoding.IsEmpty() )
// 			strEncoding = _U("ANSI");
// 		GKASSERT(0);
// 	}
// 	else if ( nTruncBeforeBytes )
// 	{
// 		nTruncBeforeBytes *= -1;
// 		m_nFileByteOffset += nTruncBeforeBytes;
// 		GKFSEEK( m_fp, m_nFileByteOffset, SEEK_SET );
// 		m_nOpFileByteLen += nTruncBeforeBytes;
// 		m_nOpFileTextLen += nTruncBeforeBytes / m_nFileCharUnitSize;
// 		GKASSERT(0);
// 	}
	return TRUE;
}

GKbool FilePos::FileReadText( GKString& strDoc )
{
	GKbool bSuccess = TRUE;
	m_strIOResult.Empty(); 
	if ( ! m_nOpFileByteLen )
	{
		return bSuccess;
	}

	// Only read up to end of file (a single read byte length cannot be over the capacity of GKint32)
	GKbool bCheckRaggedEnd = TRUE;
	GKulong nBytesRemaining = m_nFileByteLen - m_nFileByteOffset;
	if ( (GKulong)m_nOpFileByteLen >= nBytesRemaining )
	{
		m_nOpFileByteLen = (GKint32)nBytesRemaining;
		bCheckRaggedEnd = FALSE;
	}

	if ( m_nDocFlags & (CMarkup::MDF_UTF16LEFILE | CMarkup::MDF_UTF16BEFILE) )
	{
		GKint32 nUTF16Len = m_nOpFileByteLen / 2;
		GKint32 nBufferSizeForGrow = nUTF16Len + nUTF16Len/100; // extra 1%
		GKuchar* pUTF16Buffer = strDoc.GetBuffer(nBufferSizeForGrow);
		bSuccess = FileRead( pUTF16Buffer );
		if ( bSuccess && bCheckRaggedEnd )
			FileCheckRaggedEnd( (void*)pUTF16Buffer );
		strDoc.ReleaseBuffer(m_nOpFileTextLen);
	}
	else // single or multibyte file (i.e. not UTF-16)
	{
		GKchar* pBuffer = new GKchar[m_nOpFileByteLen];
		memset(pBuffer, 0, m_nOpFileByteLen);
		bSuccess = FileRead( pBuffer );
		if ( m_strEncoding.IsEmpty() )
		{
			GKint32 nNonASCII;
			GKbool bErrorAtEnd;
			m_strEncoding = _U("UTF-8");
		}
		if ( bSuccess && bCheckRaggedEnd )
			FileCheckRaggedEnd( (void*)pBuffer );
	
		GKByteString strEncoding;
		m_strEncoding.ToUTF8(strEncoding);
		strDoc.FromByteString(strEncoding.Cstr(),pBuffer);	
				
		delete [] pBuffer;
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
	}
	GKbool bSuccess = ( GKFWRITE( pConstBuffer, m_nOpFileByteLen, 1, m_fp ) == 1 );
	if ( pTempEndianBuffer )
		delete [] pTempEndianBuffer;
	if ( bSuccess )
	{
		m_nFileByteOffset += m_nOpFileByteLen;
	}
	else
		FileErrorAddResult();
	return bSuccess;
}

GKbool FilePos::FileWriteText( const GKString& strDoc, GKint32 nWriteStrLen/*=-1*/ )
{
	GKbool bSuccess = TRUE;
	 m_strIOResult .Empty();
	const GKuchar* pDoc = strDoc.Cstr();
	if ( nWriteStrLen == -1 )
		nWriteStrLen = strDoc.GetLength();
	if ( ! nWriteStrLen )
	{
		return bSuccess;
	}

	if ( m_nDocFlags & (CMarkup::MDF_UTF16LEFILE | CMarkup::MDF_UTF16BEFILE) )
	{
		m_nOpFileTextLen = nWriteStrLen;
		bSuccess = FileWrite( NULL, pDoc );
	}
	else // single or multibyte file (i.e. not UTF-16)
	{
// 		TextEncoding textencoding( _U("UTF-16"), (const void*)pDoc, nWriteStrLen );
// 		m_nOpFileTextLen = textencoding.PerformConversion( NULL, m_strEncoding );
// 		char* pMBBuffer = new char[m_nOpFileTextLen];
// 		textencoding.PerformConversion( (void*)pMBBuffer );
// 		GKASSERT(0);
// 		if ( textencoding.m_nFailedChars )
// 			GKASSERT(0);
// 		bSuccess = FileWrite( pMBBuffer );
// 		delete [] pMBBuffer;

		m_nOpFileTextLen = strDoc.GetLength();
		GKByteString strByteString;
		strDoc.ToUTF8(strByteString);
		bSuccess = FileWrite((void*)strByteString.Cstr());
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
				GKString strAppend =  str.SubString(m_nReadGatherStart,m_nReadBufferStart - m_nReadGatherStart );
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
	 m_strReadGatherMarkup .Empty();
	return nStart;
}

GKbool FilePos::FileFlush( GKString& strBuffer, GKint32 nWriteStrLen/*=-1*/, GKbool bFflush/*=FALSE*/ )
{
	GKbool bSuccess = TRUE;
	 m_strIOResult .Empty();
	if ( nWriteStrLen == -1 )
		nWriteStrLen = strBuffer .GetLength();
	if ( nWriteStrLen )
	{
		if ( (! m_nFileByteOffset) && m_strEncoding.IsEmpty() && ! strBuffer.IsEmpty() )
		{
			m_strEncoding = CMarkup::GetDeclaredEncoding( strBuffer.Cstr() );
			if ( m_strEncoding.IsEmpty() )
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
	PathPos( const GKuchar* pszPath, GKbool b ) { p=pszPath; bReader=b; i=0; iPathAttribName=0; iSave=0; nPathType=0; if (!ParsePath()) nPathType=-1; };
	GKint32 GetTypeAndInc() { i=-1; if (p) { if (p[0]=='/') { if (p[1]=='/') i=2; else i=1; } else if (p[0]) i=0; } nPathType=i+1; return nPathType; };
	GKint32 GetNumAndInc() { GKint32 n=0; while (p[i]>='0'&&p[i]<='9') n=n*10+(GKint32)p[i++]-(GKint32)'0'; return n; };
	const GKuchar* GetValAndInc() { ++i; GKuchar cEnd=']'; if (p[i]=='\''||p[i]=='\"') cEnd=p[i++]; GKint32 iVal=i; IncWord(cEnd); nLen=i-iVal; if (cEnd!=']') ++i; return &p[iVal]; };
	GKint32 GetValOrWordLen() { return nLen; };
	GKuchar GetChar() { return p[i]; };
	GKbool IsAtPathEnd() { return ((!p[i])||(iPathAttribName&&i+2>=iPathAttribName))?TRUE:FALSE; }; 
	const GKuchar* GetPtr() { return &p[i]; };
	void SaveOffset() { iSave=i; };
	void RevertOffset() { i=iSave; };
	void RevertOffsetAsName() { i=iSave; nPathType=1; };
	const GKuchar* GetWordAndInc() { GKint32 iWord=i; IncWord(); nLen=i-iWord; return &p[iWord]; };
	void IncWord() { while (p[i]&&!x_ISENDPATHWORD(p[i])) i+=GKSTRLEN(&p[i]); };
	void IncWord( GKuchar c ) { while (p[i]&&p[i]!=c) i+=GKSTRLEN(&p[i]); };
	void IncChar() { ++i; };
	void Inc( GKint32 n ) { i+=n; };
	GKbool IsAnywherePath() { return nPathType == 3; };
	GKbool IsAbsolutePath() { return nPathType == 2; };
	GKbool IsPath() { return nPathType > 0; };
	GKbool ValidPath() { return nPathType != -1; };
	const GKuchar* GetPathAttribName() { if (iPathAttribName) return &p[iPathAttribName]; return NULL; };
	GKbool AttribPredicateMatch( TokenPos& token );
private:
	GKbool ParsePath();
	GKint32 nPathType; // -1 invalid, 0 empty, 1 name, 2 absolute path, 3 anywhere path
	GKbool bReader;
	const GKuchar* p;
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
			const GKuchar* pszVal = GetValAndInc();
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

	const GKuchar* pFindEnd = NULL;
	GKint32 nNodeType = -1;
	GKint32 nEndLen = 0;
	GKint32 nName = 0;
	GKint32 nNameLen = 0;
	GKuint32 cDminus1 = 0, cDminus2 = 0;
	#define FINDNODETYPE(e,t) { pFindEnd=e; nEndLen=(sizeof(e)-1)/sizeof(GKuchar); nNodeType=t; }
	#define FINDNODETYPENAME(e,t,n) { FINDNODETYPE(e,t) nName=(GKint32)(pD-m_pDocText)+n; }
	#define FINDNODEBAD(e) { pFindEnd=_U(">"); nEndLen=1; nNodeType=-1; }

	node.nStart = m_nNext;
	node.nNodeFlags = 0;

	const GKuchar* pD = &m_pDocText[m_nNext];
	GKuint32 cD;
	while ( 1 )
	{
		cD = (GKuint32)*pD;
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
					m_pDocText =  str .Cstr();
					pD = &m_pDocText[nNewOffset];
					cD = (GKuint32)*pD; // loaded char replaces null terminator
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
					const GKuchar* pType = _U("tag");
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
				pD += GKSTRLEN( pD );
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
					const GKuchar* pEnd = pD - nEndLen + 1;
					const GKuchar* pInFindEnd = pFindEnd;
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
		pD += GKSTRLEN( pD );
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

GKbool CMarkup::SetDoc( const GKuchar* pDoc )
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

GKbool CMarkup::Load( const GKuchar* szFileName )
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	if ( ! ReadTextFile(szFileName, m_strDoc, &m_strResult, &m_nDocFlags) )
		return FALSE;
	return x_ParseDoc();
}

GKbool CMarkup::ReadTextFile( const GKuchar* szFileName, GKString& strDoc, GKString* pstrResult, GKint32* pnDocFlags, GKString* pstrEncoding )
{
	// Static utility method to load text file into strDoc
	//
	FilePos file;
	file.m_nDocFlags = (pnDocFlags?*pnDocFlags:0) | MDF_READFILE;
	GKbool bSuccess = file.FileOpen( szFileName );
	if ( pstrResult )
		*pstrResult = file.m_strIOResult;
	strDoc.Empty();
	if ( bSuccess )
	{
		file.FileSpecifyEncoding( pstrEncoding );
		file.m_nOpFileByteLen = (GKint32)((GKulong)(file.m_nFileByteLen - file.m_nFileByteOffset));
		bSuccess = file.FileReadText( strDoc );
		file.FileClose();
		if ( pstrResult )
			*pstrResult += file.m_strIOResult;
		if ( pnDocFlags )
			*pnDocFlags = file.m_nDocFlags;
	}
	return bSuccess;
}

GKbool CMarkup::Save( const GKuchar* szFileName )
{
	if ( m_nDocFlags & (MDF_READFILE|MDF_WRITEFILE) )
		return FALSE;
	return WriteTextFile( szFileName, m_strDoc, &m_strResult, &m_nDocFlags );
}

GKbool CMarkup::WriteTextFile( const GKuchar* szFileName, const GKString& strDoc, GKString* pstrResult, GKint32* pnDocFlags, GKString* pstrEncoding )
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
		if ( file.m_strEncoding.IsEmpty() && ! strDoc.IsEmpty() )
		{
			file.m_strEncoding = GetDeclaredEncoding( strDoc.Cstr() );
			if ( file.m_strEncoding.IsEmpty() )
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

GKbool CMarkup::FindElem( const GKuchar* szName )
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

GKbool CMarkup::FindChildElem( const GKuchar* szName )
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

GKString CMarkup::EscapeText( const GKuchar* szText, GKint32 nFlags )
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
	static const GKuchar* apReplace[] = { NULL,_U("&lt;"),_U("&amp;"),_U("&gt;"),_U("&quot;"),_U("&apos;") };
	GKString strText;
	const GKuchar* pSource = szText;
	GKint32 nDestSize = GKSTRLEN(pSource);
	nDestSize += nDestSize / 10 + 7;
	GKuchar* pD=strText.GetBuffer(nDestSize);GKint32 nL=0;
	GKuchar cSource = *pSource;
	GKint32 nFound;
	GKint32 nCharLen;
	while ( cSource )
	{
		if(nL+6>nDestSize){strText.ReleaseBuffer(nL);nDestSize<<=2;pD=strText.GetBuffer(nDestSize);};
		nFound = ((nFlags&MNF_ESCAPEQUOTES)?x_ISATTRIBSPECIAL(cSource):x_ISSPECIAL(cSource));
		if ( nFound )
		{
			GKbool bIgnoreAmpersand = FALSE;
			if ( (nFlags&MNF_WITHREFS) && cSource == '&' )
			{
				// Do not replace ampersand if it is start of any entity reference
				// &[#_:A-Za-zU][_:-.A-Za-z0-9U]*; where U is > 0x7f
				const GKuchar* pCheckEntity = pSource;
				++pCheckEntity;
				GKuchar c = *pCheckEntity;
				if ( x_ISSTARTENTREF(c) || ((GKuint32)c)>0x7f )
				{
					while ( 1 )
					{
						pCheckEntity += GKSTRLEN( pCheckEntity );
						c = *pCheckEntity;
						if ( c == ';' )
						{
							GKint32 nEntityLen = (GKint32)(pCheckEntity - pSource) + 1;
							GKSTRNCPY(&pD[nL],pSource,nEntityLen);nL+=GKSTRLEN(pSource);
							pSource = pCheckEntity;
							bIgnoreAmpersand = TRUE;
						}
						else if ( x_ISINENTREF(c) || ((GKuint32)c)>0x7f )
							continue;
						break;
					}
				}
			}
			if ( ! bIgnoreAmpersand )
			{
				GKSTRCPY(&pD[nL],apReplace[nFound]);nL+=GKSTRLEN(apReplace[nFound]);
			}
			++pSource; // ASCII, so 1 byte
		}
		else
		{
			nCharLen = GKSTRLEN( pSource );
			GKSTRNCPY(&pD[nL],pSource,nCharLen);nL+=GKSTRLEN(pSource);
			pSource += nCharLen;
		}
		cSource = *pSource;
	}

	strText.ReleaseBuffer(nL);
	return strText;
}

// Predefined character entities
// By default UnescapeText will decode standard HTML entities as well as the 5 in XML
// To unescape only the 5 standard XML entities, use this short table instead:
// const GKuchar* PredefEntityTable[4] =
// { _U("20060lt"),_U("40034quot"),_U("30038amp"),_U("20062gt40039apos") };
//
// This is a precompiled ASCII hash table for speed and minimum memory requirement
// Each entry consists of a 1 digit code name length, 4 digit code point, and the code name
// Each table slot can have multiple entries, table size 130 was chosen for even distribution
//
const GKuchar* PredefEntityTable[130] =
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

GKString CMarkup::UnescapeText( const GKuchar* szText, GKint32 nTextLength /*=-1*/, GKint32 nFlags /*=0*/ )
{
	// Convert XML friendly text to text as seen outside XML document
	// ampersand escape codes replaced with special characters e.g. convert "6&gt;7" to "6>7"
	// ampersand numeric codes replaced with character e.g. convert &#60; to <
	// Conveniently the result is always the same or shorter in byte length
	//
	GKString strText;
	const GKuchar* pSource = szText;
	if ( nTextLength == -1 )
		nTextLength = GKSTRLEN(szText);
	GKuchar* pD=strText.GetBuffer(nTextLength);GKint32 nL=0;
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
			while ( nCodeLen < 9 && ((GKuint32)cCodeChar) < 128 && cCodeChar != ';' )
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
					nUnicode = GKSTRTOL( &szCodeName[nNumberOffset], NULL, nBase );
				}
				else // does not start with #
				{
					// Look for matching code name in PredefEntityTable
					const GKuchar* pEntry = PredefEntityTable[x_Hash(szCodeName,sizeof(PredefEntityTable)/sizeof(const GKuchar*))];
					while ( *pEntry )
					{
						// e.g. entry: 40039apos means length 4, code point 0039, code name apos
						GKint32 nEntryLen = (*pEntry - '0');
						++pEntry;
						const GKuchar* pCodePoint = pEntry;
						pEntry += 4;
						if ( nEntryLen == nCodeLen && x_StrNCmp(szCodeName,pEntry,nEntryLen) == 0 )
						{
							// Convert digits to integer up to code name which always starts with alpha 
							nUnicode = GKSTRTOL( pCodePoint, NULL, 10 );
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
				EncodeCharUTF16( nUnicode, (unsigned short*)szChar, nCharLen );
				// Increment index past ampersand semi-colon
				if ( nUnicode ) // must check since MBCS case can clear it
				{
					GKSTRNCPY(&pD[nL],szChar,nCharLen);nL+=GKSTRLEN(szChar);
					nChar += nCodeLen + 2;
				}
			}
			if ( ! nUnicode )
			{
				// If the code is not converted, leave it as is
				pD[nL++]=_U('&');
				++nChar;
			}
		}
		else if ( bAlterWhitespace && x_ISWHITESPACE(pSource[nChar]) )
		{
			if ( nCharWhitespace == 0 && bCollapseWhitespace )
			{
				nCharWhitespace = nL;
				pD[nL++]=_U(' ');
			}
			else if ( nCharWhitespace != -1 && ! bCollapseWhitespace )
			{
				if ( nCharWhitespace == 0 )
					nCharWhitespace = nL;
				pD[nL++]=pSource[nChar];
			}
			++nChar;
		}
		else // not &
		{
			if ( bAlterWhitespace )
				nCharWhitespace = 0;
			nCharLen = GKSTRLEN(&pSource[nChar]);
			GKSTRNCPY(&pD[nL],&pSource[nChar],nCharLen);nL+=GKSTRLEN(&pSource[nChar]);
			nChar += nCharLen;
		}
	}
	if ( bAlterWhitespace && nCharWhitespace > 0 )
	{
		nL=nCharWhitespace;
	}
	strText.ReleaseBuffer(nL);
	return strText;
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

GKbool CMarkup::SavePos( const GKuchar* szPosName /*=""*/, GKint32 nMap /*=0*/ )
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
				if ( pSavedPos[nOffset].strName == (const GKuchar*)szPosName )
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

GKbool CMarkup::RestorePos( const GKuchar* szPosName /*=""*/, GKint32 nMap /*=0*/ )
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
				if ( pSavedPos[nOffset].strName == (const GKuchar*)szPosName )
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
	const GKuchar* pD = m_strDoc.Cstr();

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
		}
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
			PathPos path( strTagName.Cstr(), FALSE );
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
			GKSPRINTF( szPred, _U("[%d]"), nCount );
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

GKString CMarkup::x_GetAttrib( GKint32 iPos, const GKuchar* pAttrib ) const
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

GKbool CMarkup::x_SetAttrib( GKint32 iPos, const GKuchar* pAttrib, GKint32 nValue, GKint32 nFlags /*=0*/ )
{
	// Convert integer to string
	GKuchar szVal[25];
	GKSPRINTF( szVal, _U("%d"), nValue );
	return x_SetAttrib( iPos, pAttrib, szVal, nFlags );
}

GKbool CMarkup::x_SetAttrib( GKint32 iPos, const GKuchar* pAttrib, const GKuchar* pValue, GKint32 nFlags /*=0*/ )
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
		GKuchar* pD= strInsert.GetBuffer(nEscapedValueLen + 2 );GKint32 nL=0;
		pD[nL++]= x_ATTRIBQUOTE ;
		GKSTRNCPY(&pD[nL], strEscapedValue.Cstr(), nEscapedValueLen );nL+=GKSTRLEN( strEscapedValue.Cstr());
		pD[nL++]= x_ATTRIBQUOTE ;
		 strInsert .ReleaseBuffer(nL);
		nInsertAt = token.m_nL - ((token.m_nTokenFlags&MNF_QUOTED)?1:0);
		nReplace = token.Length() + ((token.m_nTokenFlags&MNF_QUOTED)?2:0);
	}
	else
	{
		// Insert string name value pair
		GKint32 nAttribNameLen = GKSTRLEN( pAttrib );
		GKuchar* pD= strInsert.GetBuffer(nAttribNameLen + nEscapedValueLen + 4 );GKint32 nL=0;
		pD[nL++]= ' ' ;
		GKSTRNCPY(&pD[nL], pAttrib, nAttribNameLen );nL+=GKSTRLEN( pAttrib);
		pD[nL++]= '=' ;
		pD[nL++]= x_ATTRIBQUOTE ;
		GKSTRNCPY(&pD[nL], strEscapedValue.Cstr(), nEscapedValueLen );nL+=GKSTRLEN( strEscapedValue.Cstr());
		pD[nL++]= x_ATTRIBQUOTE ;
		 strInsert .ReleaseBuffer(nL);
		nInsertAt = token.m_nNext;
	}

	GKint32 nAdjust = strInsert.GetLength() - nReplace;
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		GKint32 nNewDocLength = m_strDoc.GetLength() + nAdjust;
		m_strResult.Empty();
		if ( nNodeStart && nNewDocLength > m_pFilePos->m_nBlockSizeBasis )
		{
			GKint32 nDocCapacity = m_strDoc.GetCapacity();
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


GKbool CMarkup::x_CreateNode( GKString& strNode, GKint32 nNodeType, const GKuchar* pText )
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
		if ( GKSTRSTR(pText,_U("]]>")) != NULL )
			return FALSE;
		strNode = _U("<![CDATA[");
		strNode += pText;
		strNode += _U("]]>");
		break;
	}
	return TRUE;
}

GKString CMarkup::x_EncodeCDATASection( const GKuchar* szData )
{
	// Split CDATA Sections if there are any end delimiters
	GKString strData = _U("<![CDATA[");
	const GKuchar* pszNextStart = szData;
	const GKuchar* pszEnd = GKSTRSTR( szData, _U("]]>") );
	while ( pszEnd )
	{
		strData += GKString( pszNextStart, (GKint32)(pszEnd - pszNextStart) );
		strData += _U("]]]]><![CDATA[>");
		pszNextStart = pszEnd + 3;
		pszEnd = GKSTRSTR( pszNextStart, _U("]]>") );
	}
	strData += pszNextStart;
	strData += _U("]]>");
	return strData;
}

GKbool CMarkup::x_SetData( GKint32 iPos, GKint32 nValue )
{
	// Convert integer to string
	GKuchar szVal[25];
	GKSPRINTF( szVal, _U("%d"), nValue );
	return x_SetData( iPos, szVal, 0 );
}

GKbool CMarkup::x_SetData( GKint32 iPos, const GKuchar* szData, GKint32 nFlags )
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
			return  m_strDoc.SubString(m_nNodeOffset+4,m_nNodeLength-7 );
		else if ( m_nNodeType == MNT_PROCESSING_INSTRUCTION )
			return  m_strDoc.SubString(m_nNodeOffset+2,m_nNodeLength-4 );
		else if ( m_nNodeType == MNT_CDATA_SECTION )
			return  m_strDoc.SubString(m_nNodeOffset+9,m_nNodeLength-12 );
		else if ( m_nNodeType == MNT_TEXT )
			return UnescapeText( &(m_strDoc.Cstr())[m_nNodeOffset], m_nNodeLength, m_nDocFlags );
		else if ( m_nNodeType == MNT_LONE_END_TAG )
			return  m_strDoc.SubString(m_nNodeOffset+2,m_nNodeLength-3 );
		return  m_strDoc.SubString(m_nNodeOffset,m_nNodeLength );
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
					strData +=  m_strDoc.SubString(node.nStart+9,node.nLength-12 );
				else if ( node.nNodeType == MNT_ELEMENT )
				{
					strData.Empty();
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
						strData.Empty();
					}
					break;
				}
			}
		}
		else if ( ! pElem->iElemChild )
		{
			// Quick scan for any tags inside content
			GKint32 nContentLen = pElem->ContentLen();
			const GKuchar* pszContent = &(m_strDoc.Cstr())[nStartContent];
			const GKuchar* pszTag = GKSTRCHR( pszContent, '<' );
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
						strData +=  m_strDoc.SubString(node.nStart+9,node.nLength-12 );
				}
			}
			else // no tags
				strData = UnescapeText( &(m_strDoc.Cstr())[nStartContent], nContentLen, m_nDocFlags );
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
			return  m_strDoc.SubString(pElem->StartContent(),pElem->ContentLen() );
	}
	return _U("");
}

GKbool CMarkup::x_SetElemContent( const GKuchar* szContent )
{
	m_strResult.Empty();
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
		node.nStart += MARKUP_EOLLEN;
	if ( ! (node.nNodeFlags & MNF_WITHNOLINES) )
		node.strMeta += MARKUP_EOL;

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
			strFormat = _U(">") MARKUP_EOL;
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
			node.strMeta = MARKUP_EOL + node.strMeta;
			nInsertAt = ELEM(iPosParent).StartContent();
		}
	}
	if ( m_nDocFlags & MDF_WRITEFILE )
	{
		// Check if buffer is full
		GKint32 nNewDocLength = m_strDoc.GetLength() + node.strMeta.GetLength() - nReplace;
		GKint32 nFlushTo = node.nStart;
		 m_strResult .Empty();
		if ( bEmptyParentTag )
			nFlushTo = ELEM(iPosParent).nStart;
		if ( nFlushTo && nNewDocLength > m_pFilePos->m_nBlockSizeBasis )
		{
			GKint32 nDocCapacity = m_strDoc.GetCapacity();
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

GKbool CMarkup::x_AddElem( const GKuchar* pName, GKint32 nValue, GKint32 nFlags )
{
	// Convert integer to string
	GKuchar szVal[25];
	GKSPRINTF( szVal, _U("%d"), nValue );
	return x_AddElem( pName, szVal, nFlags );
}

GKbool CMarkup::x_AddElem( const GKuchar* pName, const GKuchar* pValue, GKint32 nFlags )
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
		GKuchar* pD= node.strMeta.GetBuffer(nLenName + 4 );GKint32 nL=0;
		pD[nL++]= '<' ;
		GKSTRNCPY(&pD[nL], pName, nLenName );nL+=GKSTRLEN( pName);
		if ( nFlags & MNF_WITHNOEND )
		{
			pD[nL++]= '>' ;
		}
		else
		{
			if ( nFlags & MNF_WITHXHTMLSPACE )
			{
				GKSTRNCPY(&pD[nL], _U(" />"), 3 );nL+=GKSTRLEN( _U(" />"));
			}
			else
			{
				GKSTRNCPY(&pD[nL], _U("/>"), 2 );nL+=GKSTRLEN( _U("/>"));
			}
		}
		 node.strMeta .ReleaseBuffer(nL);
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
		GKuchar* pD= node.strMeta.GetBuffer(pElem->nLength );GKint32 nL=0;
		pD[nL++]= '<' ;
		GKSTRNCPY(&pD[nL], pName, nLenName );nL+=GKSTRLEN( pName);
		pD[nL++]= '>' ;
		GKSTRNCPY(&pD[nL], strValue.Cstr(), nLenValue );nL+=GKSTRLEN( strValue.Cstr());
		GKSTRNCPY(&pD[nL], _U("</"), 2 );nL+=GKSTRLEN( _U("</"));
		GKSTRNCPY(&pD[nL], pName, nLenName );nL+=GKSTRLEN( pName);
		pD[nL++]= '>' ;
		 node.strMeta .ReleaseBuffer(nL);
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

GKbool CMarkup::x_AddSubDoc( const GKuchar* pSubDoc, GKint32 nFlags )
{
	if ( m_nDocFlags & MDF_READFILE || ((nFlags & MNF_CHILD) && (m_nDocFlags & MDF_WRITEFILE)) )
		return FALSE;

	m_strResult.Empty();
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
		node.strMeta.Assign(&pSubDoc[nExtractStart],nExtractLength);
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

GKbool CMarkup::x_AddNode( GKint32 nNodeType, const GKuchar* pText, GKint32 nNodeFlags )
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

GKString CMarkup::GetDeclaredEncoding( const GKuchar* szDoc )
{
	return _U("UTF-8");
}