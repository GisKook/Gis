// TestUnion.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "UnicodeStringCPP.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string>

using namespace USCPP;
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
//	UnicodeStringCPP a;
//	short m_sShortLength[7] = {1,2,3,4,5,6,7};
//	int i, a[10];
// 	memset(a, 0,10);
// 	for (i = 0; i <=10; ++i)
// 	{
// 		if(i == 10)
// 			i++;
// 		a[i+1] = 0;
// 		memset(a,0, 10);
// 	}

// 	int i = 0; 
// 	char a[1000];
// 	for(i = 0; i < 1000; i++)
// 	{
// 		a[i] = 0;
// 	}
// 	char b[1000]={'0'};
// 
// 	printf("assert \r\n");
// 	//assert(0);
// 	int sizeofulong = sizeof(long long int );

	FILE *pfile=fopen("c:/gis.txt", "rb");
	enum Charddd{
	a=0
	};
	int abc = sizeof(Charddd::a);
	string abcd("Gis_Kook");
	string b = abcd.substr(0, 1);
	
	return 0;

}

