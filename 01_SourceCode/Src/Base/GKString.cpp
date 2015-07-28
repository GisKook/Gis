#include "Base/GKString.h"
#include "unicode/unistr.h"
#include "unicode/ustring.h"
#include "unicode/ucnv.h"
#include <stdarg.h>

using namespace U_ICU_NAMESPACE;

namespace GKBASE
{
// 添加关于Format的函数,来自linux内核
#define noinline __attribute__((noinline))

#define do_div(n,base,result)\
 {\
	unsigned int __base = (base);\
	unsigned int  __rem;\
	__rem = (unsigned int)(n) % __base;\
	(n) = (unsigned int)(n) / __base;\
	result = __rem;\
}

static GKint32 skip_atoi(const GKuchar **s)
{
	GKint32 i=0;

	while (GKISDIGIT(**s))
		i = i*10 + *((*s)++) - _U('0');
	return i;
}
/* Decimal conversion is by far the most typical, and is used
 * for /proc and /sys data. This directly impacts e.g. top performance
 * with many processes running. We optimize it for speed
 * using code from
 * http://www.cs.uiowa.edu/~jones/bcd/decimal.html

static GKint32 skip_atoi(const GKuchar **s)
{
	GKint32 i=0;

	while (GKISDIGIT(**s))
		i = i*10 + *((*s)++) - _U('0');
	return i;
}
/* Decimal conversion is by far the most typical, and is used
 * for /proc and /sys data. This directly impacts e.g. top performance
 * with many processes running. We optimize it for speed
 * using code from
 * http://www.cs.uiowa.edu/~jones/bcd/decimal.html
#define do_div(n,base)\
 ({\
	unsigned int __base = (base);\
	unsigned int  __rem;\
	__rem = (unsigned int)(n) % __base;\
	(n) = (unsigned int)(n) / __base;\
	__rem;\
})

static GKint32 skip_atoi(const GKuchar **s)
{
	GKint32 i=0;

	while (GKISDIGIT(**s))
		i = i*10 + *((*s)++) - _U('0');
	return i;
}
/* Decimal conversion is by far the most typical, and is used
 * for /proc and /sys data. This directly impacts e.g. top performance
 * with many processes running. We optimize it for speed
 * using code from
 * http://www.cs.uiowa.edu/~jones/bcd/decimal.html
 * (with permission from the author, Douglas W. Jones). */

/* Formats correctly any integer in [0,99999].
 * Outputs from one to five digits depending on input.
 * On i386 gcc 4.1.2 -O2: ~250 bytes of code. */
static GKuchar* put_dec_trunc(GKuchar *buf, unsigned q)
{
	unsigned d3, d2, d1, d0;
	d1 = (q>>4) & 0xf;
	d2 = (q>>8) & 0xf;
	d3 = (q>>12);

	d0 = 6*(d3 + d2 + d1) + (q & 0xf);
	q = (d0 * 0xcd) >> 11;
	d0 = d0 - 10*q;
	*buf++ = d0 + _U('0'); /* least significant digit */
	d1 = q + 9*d3 + 5*d2 + d1;
	if (d1 != 0) {
		q = (d1 * 0xcd) >> 11;
		d1 = d1 - 10*q;
		*buf++ = d1 + _U('0'); /* next digit */

		d2 = q + 2*d2;
		if ((d2 != 0) || (d3 != 0)) {
			q = (d2 * 0xd) >> 7;
			d2 = d2 - 10*q;
			*buf++ = d2 + _U('0'); /* next digit */

			d3 = q + 4*d3;
			if (d3 != 0) {
				q = (d3 * 0xcd) >> 11;
				d3 = d3 - 10*q;
				*buf++ = d3 + _U('0');  /* next digit */
				if (q != 0)
					*buf++ = q + _U('0');  /* most sign. digit */
			}
		}
	}
	return buf;
}
/* Same with if's removed. Always emits five digits */
static GKuchar* put_dec_full(GKuchar *buf, unsigned q)
{
	/* BTW, if q is in [0,9999], 8-bit ints will be enough, */
	/* but anyway, gcc produces better code with full-sized ints */
	unsigned d3, d2, d1, d0;
	d1 = (q>>4) & 0xf;
	d2 = (q>>8) & 0xf;
	d3 = (q>>12);

	/*
	 * Possible ways to approx. divide by 10
	 * gcc -O2 replaces multiply with shifts and adds
	 * (x * 0xcd) >> 11: 11001101 - shorter code than * 0x67 (on i386)
	 * (x * 0x67) >> 10:  1100111
	 * (x * 0x34) >> 9:    110100 - same
	 * (x * 0x1a) >> 8:     11010 - same
	 * (x * 0x0d) >> 7:      1101 - same, shortest code (on i386)
	 */

	d0 = 6*(d3 + d2 + d1) + (q & 0xf);
	q = (d0 * 0xcd) >> 11;
	d0 = d0 - 10*q;
	*buf++ = d0 + _U('0');
	d1 = q + 9*d3 + 5*d2 + d1;
		q = (d1 * 0xcd) >> 11;
		d1 = d1 - 10*q;
		*buf++ = d1 + _U('0');

		d2 = q + 2*d2;
			q = (d2 * 0xd) >> 7;
			d2 = d2 - 10*q;
			*buf++ = d2 + _U('0');

			d3 = q + 4*d3;
				q = (d3 * 0xcd) >> 11; /* - shorter code */
				/* q = (d3 * 0x67) >> 10; - would also work */
				d3 = d3 - 10*q;
				*buf++ = d3 + _U('0');
					*buf++ = q + _U('0');
	return buf;
}
/* No inlining helps gcc to use registers better */
#ifdef GK_LUX
static noinline GKuchar* put_dec(GKuchar *buf, GKulong num)
#else
static GKuchar* put_dec(GKuchar *buf, unsigned num)
#endif
{
	while (1) {
		unsigned rem;
		if (num < 100000)
			return put_dec_trunc(buf, num);
		do_div(num, 100000,rem);

		buf = put_dec_full(buf, rem);
	}
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* Must be 32 == 0x20 */
#define SPECIAL	64		/* 0x */

static GKuchar *number(GKuchar *buf, GKulong num, GKint32 base, GKint32 size, GKint32 precision, GKint32 type)
{
	/* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
	static const GKuchar digits[17] = _U("0123456789ABCDEF"); /* "GHIJKLMNOPQRSTUVWXYZ"; */

	GKuchar tmp[66];
	GKuchar sign;
	GKuchar locase;
	GKint32 need_pfx = ((type & SPECIAL) && base != 10);
	GKint32 i;

	/* locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters */
	locase = (type & SMALL);
	if (type & LEFT)
		type &= ~ZEROPAD;
	sign = 0;
	if (type & SIGN) {
		if ((GKlong) num < 0) {
			sign = _U('-');
			num = - (GKlong) num;
			size--;
		} else if (type & PLUS) {
			sign = _U('+');
			size--;
		} else if (type & SPACE) {
			sign = _U(' ');
			size--;
		}
	}
	if (need_pfx) {
		size--;
		if (base == 16)
			size--;
	}

	/* generate full string in tmp[], in reverse order */
	i = 0;
	if (num == 0)
		tmp[i++] = _U('0');
	/* Generic code, for any base:
	else do {
		tmp[i++] = (digits[do_div(num,base)] | locase);
	} while (num != 0);
	*/
	else if (base != 10) { /* 8 or 16 */
		GKint32 mask = base - 1;
		GKint32 shift = 3;
		if (base == 16) shift = 4;
		do {
			tmp[i++] = (digits[(num) & mask] | locase);
			num >>= shift;
		} while (num);
	} else { /* base 10 */
		i = put_dec(tmp, num) - tmp;
	}

	/* printing 100 using %2d gives "100", not "00" */
	if (i > precision)
		precision = i;
	/* leading space padding */
	size -= precision;
	if (!(type & (ZEROPAD+LEFT)))
		while(--size >= 0)
			*buf++ = _U(' ');
	/* sign */
	if (sign)
		*buf++ = sign;
	/* "0x" / "0" prefix */
	if (need_pfx) {
		*buf++ = _U('0');
		if (base == 16)
			*buf++ = (_U('X') | locase);
	}
	/* zero or space padding */
	if (!(type & LEFT)) {
		GKuchar c = (type & ZEROPAD) ? _U('0') : _U(' ');
		while (--size >= 0)
			*buf++ = c;
	}
	/* hmm even more zero padding? */
	while (i <= --precision)
		*buf++ = _U('0');
	/* actual digits of result */
	while (--i >= 0)
		*buf++ = tmp[i];
	/* trailing space padding */
	while (--size >= 0)
		*buf++ = _U(' ');
	return buf;
}

GKint32 strnlen(const GKuchar * s, GKint32 count)
{
	const GKuchar *sc;

	for (sc = s; count-- && *sc != _U('\0'); ++sc)
		/* nothing */;
	return sc - s;
}

static GKuchar *string(GKuchar *buf, GKuchar *s, GKint32 field_width, GKint32 precision, GKint32 flags)
{
	GKint32 len, i;

	if (s == 0)
		s = _U("<NULL>");

	len = strnlen(s, precision);

	if (!(flags & LEFT))
		while (len < field_width--)
			*buf++ = _U(' ');
	for (i = 0; i < len; ++i)
		*buf++ = *s++;
	while (len < field_width--)
		*buf++ = _U(' ');
	return buf;
}

GKint32 GKvswprintf(GKuchar *buf, const GKuchar *fmt, va_list args)
{
	GKulong num;
	GKint32 base;
	GKuchar *str;

	GKint32 flags;		/* flags to number() */

	GKint32 field_width;	/* width of output field */
	GKint32 precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	GKint32 qualifier;		/* _U('h'), _U('l'), or _U('L') for integer fields */
				/* _U('z') support added 23/7/1999 S.H.    */
				/* _U('z') changed to _U('Z') --davidm 1/25/99 */
				/* _U('t') added for ptrdiff_t */
	str = buf;
	for (; *fmt ; ++fmt) {
		if (*fmt != _U('%')) {
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first _U('%') */
			switch (*fmt) {
				case _U('-'): flags |= LEFT; goto repeat;
				case _U('+'): flags |= PLUS; goto repeat;
				case _U(' '): flags |= SPACE; goto repeat;
				case _U('#'): flags |= SPECIAL; goto repeat;
				case _U('0'): flags |= ZEROPAD; goto repeat;
			}

		/* get field width */
		field_width = -1;
		if (GKISDIGIT(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == _U('*')) {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, GKint32);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == _U('.')) {
			++fmt;
			if (GKISDIGIT(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == _U('*')) {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, GKint32);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == _U('h') || *fmt == _U('l') || *fmt == _U('L') ||
		    *fmt == _U('Z') || *fmt == _U('z') || *fmt == _U('t')) {
			qualifier = *fmt;
			++fmt;
			if (qualifier == _U('l') && *fmt == _U('l')) {
				qualifier = _U('L');
				++fmt;
			}
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case _U('c'):
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = _U(' ');
			*str++ = (GKuchar) va_arg(args, GKint32);
			while (--field_width > 0)
				*str++ = _U(' ');
			continue;

		case _U('s'):
			str = string(str, va_arg(args, GKuchar *), field_width, precision, flags);
			continue;
#if 0
		case _U('p'):
			str = pointer(fmt+1, str,
					va_arg(args, void *),
					field_width, precision, flags);
			/* Skip all alphanumeric pointer suffixes */
			while (isalnum(fmt[1]))
				fmt++;
			continue;
#endif
		case _U('n'):
			if (qualifier == _U('l')) {
				long * ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				GKint32 * ip = va_arg(args, GKint32 *);
				*ip = (str - buf);
			}
			continue;

		case _U('%'):
			*str++ = _U('%');
			continue;

		/* integer number formats - set up the flags and "break" */
		case _U('o'):
			base = 8;
			break;

		case _U('x'):
			flags |= SMALL;
		case _U('X'):
			base = 16;
			break;

		case _U('d'):
		case _U('i'):
			flags |= SIGN;
		case _U('u'):
			break;

		default:
			*str++ = _U('%');
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}
		if (qualifier == _U('L'))  /* "quad" for 64 bit variables */
			num = va_arg(args, unsigned long long);
		else if (qualifier == _U('l')) {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long) num;
		} else if (qualifier == _U('Z') || qualifier == _U('z')) {
			num = va_arg(args, GKint32);
		} else if (qualifier == _U('t')) {
			num = va_arg(args, GKint32);
		} else if (qualifier == _U('h')) {
			num = (unsigned short) va_arg(args, GKint32);
			if (flags & SIGN)
				num = (signed short) num;
		} else {
			num = va_arg(args, GKint32);
			if (flags & SIGN)
				num = (GKint32) num;
		}
		str = number(str, num, base, field_width, precision, flags);
	}
	*str = '\0';
	return str-buf;
}


GKString::GKString()
{
	m_pString = new(&m_Context) UnicodeString;
}

GKString::GKString( const GKString& str )
{
	m_pString = new(&m_Context) UnicodeString(*str.m_pString);
}

GKString::GKString( const GKuchar* pStr )
{
	m_pString = new(&m_Context) UnicodeString((UChar*)pStr);
}

GKString::GKString( const GKuchar* pStr, GKint32 nLength )
{
	m_pString = new(&m_Context) UnicodeString((UChar*)pStr, nLength);
}

GKString::GKString( const UnicodeString& str )
{
	m_pString = new(&m_Context) UnicodeString(str);
}

GKString& GKString::operator=( const GKString& str)
{
	if (*this == str)
	{
		return *this;
	}
	*this->m_pString = *str.Custr();

	return *this;
}

GKString& GKString::operator=( const GKuchar* pStr )
{
	if (*m_pString == UnicodeString((UChar*)pStr))
	{
		return *this;
	}
	*m_pString = UnicodeString((UChar*)pStr);

	return *this;
}

GKbool GKString::operator==( const GKString& str )
{
	return m_pString == str.m_pString;
}

GKbool GKString::operator==( const GKuchar* pStr )
{
	return (GKbool)(*m_pString == UnicodeString((UChar*)pStr));
}

GKString& GKString::operator+=( const GKString& str )
{
	*m_pString += *str.m_pString;

	return *this;
}

GKString& GKString::operator+=( const GKuchar* pStr )
{
	*m_pString += UnicodeString((UChar*) pStr);

	return *this;
}

const GKuchar* GKString::Cstr() const
{
	return (GKuchar*)m_pString->getTerminatedBuffer();
}

GKint32 GKString::GetLength() const
{
	return m_pString->length();
}

GKuchar* GKString::GetBuffer( GKint32 nLength )
{
	return (GKuchar*)m_pString->getBuffer(nLength);
}

void GKString::ReleaseBuffer(GKint32 nLength)
{
	m_pString->releaseBuffer(nLength);	
}

void GKString::Empty()
{
	m_pString->remove();
}

void GKString::Assign( const GKuchar* pStr, GKint32 nAssignLen )
{
	GKASSERT(pStr != NULL);
	memcpy(m_pString->getBuffer(nAssignLen), pStr, nAssignLen*sizeof(GKuchar));
	m_pString->releaseBuffer();
}

void GKString::Assign( const GKchar* pStr )
{
	GKASSERT(pStr != NULL);
	GKint32 nLen = strlen(pStr);
	// 考虑到一个字符在本地的编码可能转换为4个Unicode编码，所以这里用了4
	u_uastrcpy(m_pString->getBuffer(nLen*4), pStr);
	m_pString->releaseBuffer(nLen*4);	
}

void GKString::Assign( const GKchar* pStr, GKint32 nLen )
{
	GKASSERT(pStr != NULL);
	// 考虑到一个字符在本地的编码可能转换为4个Unicode编码，所以这里用了4
	u_uastrncpy(m_pString->getBuffer(nLen*4), pStr, nLen);
	m_pString->releaseBuffer(nLen*4);
}
GKbool GKString::operator!=( const GKString& str )
{
	return !(*this == str);
}

GKbool GKString::operator<(const GKString& str) const
{
	return m_pString < str.m_pString;
}

GKbool GKString::operator>=(const GKString& str) const
{
	return !operator<(str);
}

GKbool GKString::IsEmpty() const
{
	return m_pString->isEmpty();
}

GKString GKString::SubString( GKint32 nStartPos, GKint32 nLen ) const
{
	return GKString(m_pString->setTo(*this->Custr(), nStartPos, nLen));
}

const UnicodeString* GKString::Custr() const 
{
	return m_pString;
}

GKint32 GKString::GetCapacity() const
{
	return m_pString->getCapacity();
}

void GKString::FromUTF8( const GKchar* str ) const
{
	*m_pString = m_pString->fromUTF8(str);
}

GKByteString& GKString::ToUTF8( GKByteString& str ) const
{
	m_pString->toUTF8String(str.m_string);
	return str;	
}

GKString& GKString::FromByteString( const GKchar* pCharset, const GKchar* pSrc)
{
	if (GKSTRCMPA(pCharset, "UTF-8")==0)
	{
		m_pString->fromUTF8(pSrc);
		return *this;
	}
	GKint32 nLen = GKSTRLENA(pSrc);
	GKint32 nTargetLen = nLen*3;
	GKchar* target = new GKchar[nTargetLen];
	memset(target, 0, nTargetLen);
	UErrorCode nErrorCode = U_ZERO_ERROR;
	ucnv_convert("UTF16", pCharset, (char*)target, nTargetLen, pSrc, nLen,&nErrorCode);
	GKASSERT(nErrorCode == 0);
	GKuchar* pBuffer = m_pString->getBuffer(nTargetLen);
	memcpy(pBuffer, target, nTargetLen);
	m_pString->releaseBuffer(nTargetLen);

	return *this;
}

GKString GKString::Format( const GKuchar* fmt, ... )
{
	va_list argptr; 
	va_start(argptr,fmt); 
	const GKuint32 MAXCHARTERCOUNT=1024; // 最大的限制
	GKuchar buf[MAXCHARTERCOUNT];
	memset(buf, 0, MAXCHARTERCOUNT*2);
	GKint32 nSize= GKvswprintf(buf,fmt,argptr); 
	va_end(argptr);

	return GKString(buf, nSize);
}

GKBASE::GKuchar GKString::operator[]( GKint32 index )
{
	return (*m_pString)[0];
}

GKString & GKString::Replace( GKString oldstring, GKString newstring )
{
	m_pString->findAndReplace(*oldstring.m_pString, *newstring.m_pString);

	return *this;
}


StringPiece::StringPiece(const GKuchar* str)
: m_pStr(str), m_nLength((str == NULL) ? 0 : static_cast<GKint32>(GKSTRLEN(str))) { }

StringPiece::StringPiece(const StringPiece& x, GKint32 pos) {
	if (pos < 0) {
		pos = 0;
	} else if (pos > x.m_nLength) {
		pos = x.m_nLength;
	}
	m_pStr = x.m_pStr + pos;
	m_nLength = x.m_nLength - pos;
}

StringPiece::StringPiece(const StringPiece& x, GKint32 pos, GKint32 len) {
	if (pos < 0) {
		pos = 0;
	} else if (pos > x.m_nLength) {
		pos = x.m_nLength;
	}
	if (len < 0) {
		len = 0;
	} else if (len > x.m_nLength - pos) {
		len = x.m_nLength - pos;
	}
	m_pStr = x.m_pStr + pos;
	m_nLength = len;
}

void StringPiece::Set(const GKuchar* str) {
	m_pStr = str;
	if (str != NULL)
		m_nLength = static_cast<GKint32>(GKSTRLEN(str));
	else
		m_nLength = 0;
}

BASE_API GKbool 
operator==(const StringPiece& x, const StringPiece& y) {
	GKint32 len = x.Size();
	if (len != y.Size()) {
		return false;
	}
	const GKuchar* p = x.Data();
	const GKuchar* p2 = y.Data();
	// Test last byte in case strings share large common prefix
	if ((len > 0) && (p[len-1] != p2[len-1])) return false;
	// At this point we can, but don't have to, ignore the last byte.
	return GKMEMCMP(p, p2, 2*(len-1)) == 0;
}


/* Microsft Visual Studios <= 8.0 complains about redefinition of this
* static const class variable. However, the C++ standard states that this 
* definition is correct. Perhaps there is a bug in the Microsoft compiler. 
* This is not an issue on any other compilers (that we know of) including 
* Visual Studios 9.0.
* Cygwin with MSVC 9.0 also complains here about redefinition.
*/
#if (!defined(_MSC_VER) || (_MSC_VER >= 1500)) && !defined(CYGWINMSVC)
const GKint32 StringPiece::npos = 0x7fffffff;
#endif

extern GKint32 GKStrLen( const GKuchar* str )
{
	return u_strlen(str);
}

extern GKint32 GKStrCmp( GKuchar* str1, GKuchar* str2 )
{
	return u_strcmp(str1, str2);
}

extern GKint32 GKStrCmpNoCase( GKuchar* str1, GKuchar* str2 )
{
	GKuchar  c1, c2;

	for(;;) 
	{
		c1=*str1++;
		c2=*str2++;
		if (c1 != c2)
		{
			if (c1>='a' && c1 <= 'z') c1 -= 'a'-'A'; // toUpper
			if (c2>='a' && c2 <= 'z') c2 -= 'a'-'A'; // toUpper
			if (c1 != c2 || c1 == 0)break;
		}
	}
	return (GKint32)c1 - (GKint32)c2;
}

extern GKString operator+( const GKString& str1, const GKString& str2 )
{
	return GKString(*str1.Custr()+ *str2.Custr());
}

}