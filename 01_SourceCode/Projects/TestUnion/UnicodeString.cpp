#include "stdafx.h"
#include "UnicodeString.h"
#include <cstdio>

using namespace std;

MUSGISKOOK::MyUnicodeString::MyUnicodeString()
{
	printf("Construction for MyUnicodeString\r\n");
	m_pTemp = (void*)0x12345678;
	m_nCapacity = 1024;
	m_nLength = 768;
	m_pData = (short*)0x87654321;
	m_sFlag = 256;
}

MUSGISKOOK::MyUnicodeString::~MyUnicodeString()
{
	printf("DeConstruaction for MyUnicodeString\r\n");
}