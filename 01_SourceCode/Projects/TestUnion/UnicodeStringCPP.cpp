#include "stdafx.h"
#include "UnicodeStringCPP.h"
#include "UnicodeString.h"
#include <cstdio>

using namespace std;
using namespace USCPP;
using namespace MUSGISKOOK;

UnicodeStringCPP::UnicodeStringCPP()
{
	printf("Construction For UnicodeString\r\n");
	m_pMyUnicodeString = new MyUnicodeString;
}

UnicodeStringCPP::~UnicodeStringCPP()
{
	printf("DeConstruction For UnicodeString\r\n");
}